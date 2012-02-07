/*
 * Copyright (C) 2012  Politecnico di Milano
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "yamca_schedpol.h"

#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <thread>

#include "bbque/modules_factory.h"
#include "bbque/app/working_mode.h"
#include "bbque/app/overheads.h"
#include "bbque/plugins/logger.h"


/** Metrics (class VALUE) declaration */
#define YAMCA_VALUE_METRIC(NAME, DESC)\
 {SCHEDULER_MANAGER_NAMESPACE".yamca."NAME, DESC, MetricsCollector::VALUE, 0}
/** Increase value for the specified metric */
#define YAMCA_ADD_VALUE(METRICS, INDEX, AMOUNT) \
	mc.Add(METRICS[INDEX].mh, AMOUNT);
/** Reset value for the specified metric */
#define YAMCA_RESET_VALUE(METRICS, INDEX) \
	mc.Reset(METRICS[INDEX].mh);

/** Metrics (class SAMPLE) declaration */
#define YAMCA_SAMPLE_METRIC(NAME, DESC)\
 {SCHEDULER_MANAGER_NAMESPACE".yamca."NAME, DESC, MetricsCollector::SAMPLE, 0}
/** Reset the timer used to evaluate metrics */
#define YAMCA_RESET_TIMING(TIMER) \
	TIMER.start();
/** Acquire a new completion time sample */
#define YAMCA_GET_TIMING(METRICS, INDEX, TIMER) \
	mc.AddSample(METRICS[INDEX].mh, TIMER.getElapsedTimeMs());

/* Get a new samplefor the metrics */
#define YAMCA_GET_SAMPLE(METRICS, INDEX, VALUE) \
	mc.AddSample(METRICS[INDEX].mh, VALUE);


#define SCHED_MAP_ESTIMATION\
	((sizeof(float) + sizeof(SchedEntity_t))*sched_map.size() +\
	 sizeof(sched_map))

namespace bu = bbque::utils;

namespace bbque { namespace plugins {

/* Definition of metrics used by this module */
MetricsCollector::MetricsCollection_t
YamcaSchedPol::coll_metrics[YAMCA_METRICS_COUNT] = {
	//-----  Value metrics
	YAMCA_SAMPLE_METRIC("map",
			"Size of the sched-entity map per cluster [bytes]"),
	YAMCA_SAMPLE_METRIC("entities",
			"Number of entity to schedule per cluster"),
	//----- Timing metrics
	YAMCA_SAMPLE_METRIC("ord", "Time to order SchedEntity into a cluster [ms]"),
	YAMCA_SAMPLE_METRIC("mcomp", "Time for computing a single metrics [ms]"),
	YAMCA_SAMPLE_METRIC("sel", "Time to assign AWMs to EXCs of a cluster [ms]")
};


YamcaSchedPol::YamcaSchedPol():
	rsrc_acct(ResourceAccounter::GetInstance()),
	mc(bu::MetricsCollector::GetInstance()) {

	// Get a logger
	plugins::LoggerIF::Configuration conf(
			SCHEDULER_MANAGER_NAMESPACE"."
			SCHEDULER_POLICY_NAME);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

	if (logger)
		logger->Info("YaMCA: Built a new dynamic object[%p]\n", this);
	else
		std::cout << "YaMCA: Build new dynamic object ["
			<< this << "]" << std::endl;

	// Resource view counter
	tok_counter = 0;

	// Register all the metrics to collect
	mc.Register(coll_metrics, YAMCA_METRICS_COUNT);
}


YamcaSchedPol::~YamcaSchedPol() {
}


//----- Scheduler policy module interface

char const * YamcaSchedPol::Name() {
	return SCHEDULER_POLICY_NAME;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::Schedule(
		bbque::SystemView & sv, RViewToken_t & rav) {
	ExitCode_t result;

	logger->Debug(
			"<<<<<<<<<<<<<<<<< Scheduling policy starting >>>>>>>>>>>>>>>>>>");
	// Get a resources view from Resource Accounter
	if (InitResourceView() == SCHED_ERROR) {
		logger->Fatal("Schedule: Aborted due to resource state view missing");
		return SCHED_ERROR;
	}

	// Get the number of clusters
	num_clusters = sv.ResourceTotal(RSRC_CLUSTER);
	clusters_full.resize(num_clusters);
	clusters_full = { false };

	logger->Info("Schedule: Found %d clusters on the platform.", num_clusters);
	logger->Info("lowest prio = %d", sv.ApplicationLowestPriority());

	// Iterate from the highest to the lowest priority applications queue
	for (AppPrio_t prio = 0; prio <= sv.ApplicationLowestPriority();
			++prio) {

		if (!sv.HasApplications(prio))
			continue;

		// Schedule applications with priority == prio
		result = SchedulePrioQueue(sv, prio);
		if (result != SCHED_OK) {
			rsrc_acct.PutView(rsrc_view_token);
			return result;
		}
	}

	logger->Debug(
			">>>>>>>>>>>>>>>>> Scheduling policy exiting <<<<<<<<<<<<<<<<<<<");

	rsrc_acct.PrintStatusReport(rsrc_view_token);

	rav = rsrc_view_token;
	return SCHED_DONE;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::InitResourceView() {
	// Set the counter (avoiding overflow)
	tok_counter == std::numeric_limits<uint32_t>::max() ?
		tok_counter = 0:
		++tok_counter;

	// Build a string path for the resource state view
	std::string schedpolname(
			SCHEDULER_POLICY_NAMESPACE
			SCHEDULER_POLICY_NAME);
	char token_path[30];
	snprintf(token_path, 30, "%s%d", schedpolname.c_str(), tok_counter);

	// Get a resource state view
	ResourceAccounter::ExitCode_t view_result;
	view_result = rsrc_acct.GetView(token_path, rsrc_view_token);
	if (view_result != ResourceAccounter::RA_SUCCESS) {
		logger->Fatal("Init: Cannot get a resource state view");
		return SCHED_ERROR;
	}

	logger->Debug("Init: Requiring view token for %s", token_path);
	logger->Debug("Init: Resources state view token = %d", rsrc_view_token);
	return SCHED_OK;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::SchedulePrioQueue(
		bbque::SystemView & sv,
		AppPrio_t prio) {
	ExitCode_t result;


	//Order scheduling entities
	for (uint16_t cl_id = 0; cl_id < num_clusters; ++cl_id) {
		SchedEntityMap_t sched_map;
		logger->Debug("Schedule: ======================= Cluster%d :", cl_id);

		// Skip current cluster if full
		if (clusters_full[cl_id]) {
			logger->Warn("Schedule: cluster %d is full, skipping...", cl_id);
			continue;
		}

		YAMCA_RESET_TIMING(yamca_tmr);

		// Order schedule entities by metrics
		result = OrderSchedEntity(sched_map, sv, prio, cl_id);
		if (result == SCHED_CLUSTER_FULL) {
			clusters_full[cl_id] = true;
			continue;
		}

		YAMCA_GET_TIMING(coll_metrics, YAMCA_ORDER_TIME, yamca_tmr);

		// Nothing to schedule in this cluster
		if (sched_map.empty())
			continue;

		if (result != SCHED_OK)
			return result;

		YAMCA_GET_SAMPLE(coll_metrics, YAMCA_SCHEDMAP_SIZE,
				SCHED_MAP_ESTIMATION);
		YAMCA_GET_SAMPLE(coll_metrics, YAMCA_NUM_ENTITY, sched_map.size());

		YAMCA_RESET_TIMING(yamca_tmr);

		// For each application schedule a working mode
		SelectWorkingModes(sched_map);

		YAMCA_GET_TIMING(coll_metrics, YAMCA_SELECT_TIME, yamca_tmr);
	}

	return SCHED_OK;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::OrderSchedEntity(
		SchedEntityMap_t & sched_map,
		bbque::SystemView & sv,
		AppPrio_t prio,
		int cl_id) {
	AppsUidMapIt app_it;
	AppPtr_t papp;

	// Applications to be scheduled
	papp = sv.GetFirstWithPrio(prio, app_it);
	for ( ; papp; papp = sv.GetNextWithPrio(prio, app_it)) {

		// Check a set of conditions accordingly to skip current
		// application/EXC
		if (CheckSkipConditions(papp))
			continue;

		// Compute the metrics for all the working modes.
		ExitCode_t result = InsertWorkingModes(sched_map, papp, cl_id);
		if (result == SCHED_SKIP_APP)
			continue;

		if (result != SCHED_OK)
			return result;
	}

	return SCHED_OK;
}


void YamcaSchedPol::SelectWorkingModes(SchedEntityMap_t & sched_map) {
	Application::ExitCode_t app_result;
	logger->Debug(
			"____________________| Scheduling entities |____________________");

	// The scheduling entities should be picked in a descending order of
	// metrics value
	SchedEntityMap_t::reverse_iterator se_it(sched_map.rbegin());
	SchedEntityMap_t::reverse_iterator end_se(sched_map.rend());

	// Pick the entity and set the new Application Working Mode
	for (; se_it != end_se; ++se_it) {
		AppPtr_t & papp = (se_it->second).first;
		AwmPtr_t const & eval_awm((se_it->second).second);

		// Check a set of conditions accordingly to skip current
		// application/EXC
		if (CheckSkipConditions(papp))
			continue;

		logger->Debug("Selecting: [%s] schedule request for AWM{%d}...",
				papp->StrId(),
				eval_awm->Id());

		// Schedule the application in the working mode just evaluated
		app_result = papp->ScheduleRequest(eval_awm, rsrc_view_token);

		// Debugging messages
		if (app_result != Application::APP_WM_ACCEPTED) {
			logger->Debug("Selecting: [%s] AWM{%d} rejected ! [ret %d]",
							papp->StrId(),
							eval_awm->Id(),
							app_result);
			continue;
		}

		if (!papp->Synching() || papp->Blocking()) {
			logger->Debug("Selecting: [%s] in %s/%s", papp->StrId(),
					Application::StateStr(papp->State()),
					Application::SyncStateStr(papp->SyncState()));
			continue;
		}

		AwmPtr_t const & new_awm = papp->NextAWM();
		logger->Info("Selecting: [%s] set to AWM{%d} on clusters map [%s]",
					papp->StrId(),
					new_awm->Id(),
					new_awm->ClusterSet().to_string().c_str());
	}
}


inline bool YamcaSchedPol::CheckSkipConditions(AppPtr_t const & papp) {

	// Skip if the application has been rescheduled yet (with success) or
	// disabled in the meanwhile
	if (!papp->Active() && !papp->Blocking()) {
		logger->Debug("Skipping [%s]. State = {%s/%s}",
					papp->StrId(),
					Application::StateStr(papp->State()),
					Application::SyncStateStr(papp->SyncState()));
		return true;
	}

	// Avoid double AWM selection for RUNNING applications with an already
	// assigned AWM.
	if ((papp->State() == Application::RUNNING) && papp->NextAWM()) {
		logger->Debug("Skipping [%s]. No reconfiguration needed. (AWM=%d)",
				papp->StrId(), papp->CurrentAWM()->Id());
		return true;
	}

	return false;
}


void join_thread(std::thread & t) {
	t.join();
}

SchedulerPolicyIF::ExitCode_t YamcaSchedPol::InsertWorkingModes(
		SchedEntityMap_t & sched_map,
		AppPtr_t const & papp,
		int cl_id) {
	std::list<std::thread> awm_thds;

	// Working modes
	AwmPtrList_t const * awms = papp->WorkingModes();
	AwmPtrList_t::const_iterator awm_it(awms->begin());
	AwmPtrList_t::const_iterator end_awm(awms->end());

	for (; awm_it != end_awm; ++awm_it) {

		awm_thds.push_back(
			std::thread(&YamcaSchedPol::EvalWorkingMode, this,
			&sched_map, papp, (*awm_it), cl_id)
		);
	}

	for_each(awm_thds.begin(), awm_thds.end(), join_thread);
	awm_thds.clear();

	logger->Debug("Schedule table size = %d", sched_map.size());
	return SCHED_OK;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::EvalWorkingMode(
				SchedEntityMap_t * sched_map,
				AppPtr_t const & papp,
				AwmPtr_t const & wm,
				int cl_id) {
	std::unique_lock<std::mutex> sched_ul(sched_mtx, std::defer_lock);

	logger->Debug("Insert: [%s] AWM{%d} metrics computing...", papp->StrId(),
			wm->Id());

	// Skip if the application has been disabled/stopped in the meanwhile
	if (papp->Disabled()) {
		logger->Debug("Insert: [%s] disabled/stopped during scheduling [Ord]",
				papp->StrId());
		return SCHED_SKIP_APP;
	}

	// Metrics computation
	float metrics;
	ExitCode_t result = MetricsComputation(papp, wm, cl_id, metrics);

	switch (result) {
	case SCHED_CLUSTER_FULL:
		logger->Warn("Insert: No more PEs in cluster %d", cl_id);
		return result;

	case SCHED_RSRC_UNAV:
		logger->Warn("Insert: [%s] AWM{%d} CL=%d unavailable resources "
				"[RA:%d]", papp->StrId(), wm->Id(), cl_id, result);
		return result;

	case SCHED_ERROR:
		logger->Error("Insert: An error occurred [ret %d]", result);
		return result;

	default:
		break;
	}

	// Insert the SchedEntity in the map ordered by the metrics value
	sched_ul.lock();
	sched_map->insert(std::pair<float, SchedEntity_t>(metrics,
				SchedEntity_t(papp, wm)));

	logger->Info("{%d} Insert: [%s] AWM{%d} CL=%d metrics %.4f",
					sched_map->size(), papp->StrId(),
					wm->Id(), cl_id, metrics);

	return SCHED_OK;
}




// Functions used by MetricsComputation() to get the migration and
// reconfiguration overhead
float GetMigrationOverhead(AppPtr_t const & papp, AwmPtr_t const & wm,
		int cl_id);
float GetReconfigOverhead(AppPtr_t const & papp, AwmPtr_t const & wm);


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::MetricsComputation(
		AppPtr_t const & papp,
		AwmPtr_t const & wm,
		int cl_id,
		float & metrics) {
	ExitCode_t result;
	metrics = 0.0;

	Timer comp_tmr;
	YAMCA_RESET_TIMING(comp_tmr);

	// If the resource binding implies migration from a cluster to another we
	// have to evaluate the overheads
	float migr_cost = GetMigrationOverhead(papp, wm, cl_id);

	// If the working mode is different from the current one, the Execution
	// Context should be reconfigured. Let's estimate the overhead.
	float reconf_cost = GetReconfigOverhead(papp, wm);

	// Contention level
	float cont_level;
	result = GetContentionLevel(papp, wm, cl_id, cont_level);
	if (result != SCHED_OK)
		return result;

	// Metrics
	logger->Debug("AWM value: %.2f", wm->Value());
	metrics = (wm->Value() - reconf_cost - migr_cost) / cont_level;

	YAMCA_GET_TIMING(coll_metrics, YAMCA_METCOMP_TIME, comp_tmr);
	return SCHED_OK;
}


inline float GetMigrationOverhead(AppPtr_t const & papp,
		AwmPtr_t const & wm,
		int cl_id) {
	// Silence args warnings
	(void) papp;
	(void) wm;
	(void) cl_id;

	//TODO:  To implement
	//logger->Debug("Migration overhead: 0.0");
	return 0.0;
}


inline float GetReconfigOverhead(AppPtr_t const & papp,
		AwmPtr_t const & wm) {
	// Silence args warnings
	(void) papp;
	(void) wm;

//	logger->Debug("Reconfiguration overhead : 0.0");
	return 0.0;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::GetContentionLevel(
		AppPtr_t const & papp,
		AwmPtr_t const & wm,
		int cl_id,
		float & cont_level) {
	WorkingMode::ExitCode_t wm_result;

	// Safety data check
	if (!wm) {
		logger->Crit("Contention level: Missing working mode.\n"
				"Possibile data corruption in %s",
				SCHEDULER_POLICY_NAMESPACE"yamca");
		assert(!wm);
		return SCHED_ERROR;
	}

	// Binding of the resources requested by the working mode into the current
	// cluster. Note: No multi-cluster allocation supported yet!
	logger->Debug("Contention level: Binding into cluster %d", cl_id);
	wm_result = wm->BindResource("cluster", RSRC_ID_ANY, cl_id);
	if (wm_result == WorkingMode::WM_RSRC_MISS_BIND)
		logger->Error("Contention level: {AWM %d} [cluster = %d]"
				"Incomplete resources binding. %d / %d resources bound.",
						wm->Id(), cl_id, wm->GetSchedResourceBinding()->size(),
						wm->RecipeResourceUsages().size());

	// Contention level
	return ComputeContentionLevel(papp, wm->GetSchedResourceBinding(),
			cont_level);
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::ComputeContentionLevel(
		AppPtr_t const & papp,
		UsagesMapPtr_t const & rsrc_usages,
		float & cont_level) {
	uint64_t rsrc_avail;
	uint64_t min_usage;
	cont_level = 0;

	// Check the availability of the resources requested
	UsagesMap_t::const_iterator usage_it(rsrc_usages->begin());
	UsagesMap_t::const_iterator end_usage(rsrc_usages->end());
	while (usage_it != end_usage) {
		// Current resource
		std::string const & rsrc_path(usage_it->first);
		UsagePtr_t const & pusage(usage_it->second);

		// Query resource availability
		rsrc_avail = rsrc_acct.Available(pusage->GetBindingList(),
				rsrc_view_token, papp);
		logger->Debug("{%s} availability = %llu", rsrc_path.c_str(),
				rsrc_avail);

		// Is the request satisfiable?
		if (rsrc_avail < pusage->GetAmount()) {
			logger->Debug("Contention level: [%s] R=%d / A=%d",
					rsrc_path.c_str(), pusage->GetAmount(),	rsrc_avail);

			// Set the availability to a 1/10 of the requested amount of
			// resource in order to increase dramatically the resulting
			// contention level
			rsrc_avail = 0.1 * pusage->GetAmount();
		}

		// Get the resource usage of the AWM with the min value
		AwmPtr_t wm_min(papp->LowValueAWM());
		min_usage = wm_min->ResourceUsageAmount(PathTemplate(rsrc_path));

		// Update the contention level (inverse)
		cont_level +=
			(((float) pusage->GetAmount()) * min_usage) / (float) rsrc_avail;

		++usage_it;
	}

	// Avoid division by zero (in the caller)
	if (cont_level == 0)
		cont_level = 0.1;

	logger->Debug("Contention level: %.4f", cont_level);
	return SCHED_OK;
}

//----- static plugin interface

void * YamcaSchedPol::Create(PF_ObjectParams *) {
	return new YamcaSchedPol();
}


int32_t YamcaSchedPol::Destroy(void * plugin) {
  if (!plugin)
    return -1;
  delete (YamcaSchedPol *)plugin;
  return 0;
}


} // namesapce plugins

} // namespace bque

