/**
 *       @file  yamca_schedpol.cc
 *      @brief  The YaMCA resource scheduler (dynamic plugin)
 *
 * This implements a dynamic C++ plugin which implements the YaMCA resource
 * scheduler heuristic.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "yamca_schedpol.h"

#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iostream>

#include "bbque/modules_factory.h"
#include "bbque/app/working_mode.h"
#include "bbque/app/overheads.h"
#include "bbque/plugins/logger.h"


namespace bbque { namespace plugins {


YamcaSchedPol::YamcaSchedPol():
	rsrc_acct(ResourceAccounter::GetInstance()) {

	// Get a logger
	plugins::LoggerIF::Configuration conf(
			SCHEDULER_POLICY_NAMESPACE
			SCHEDULER_POLICY_NAME);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

	if (logger)
		logger->Info("YaMCA: Built a new dynamic object[%p]\n", this);
	else
		std::cout << "YaMCA: Build new dynamic object ["
			<< this << "]" << std::endl;

	// Resource view counter
	tok_counter = 0;
}


YamcaSchedPol::~YamcaSchedPol() {
}


//----- Scheduler policy module interface

char const * YamcaSchedPol::Name() {
	return SCHEDULER_POLICY_NAME;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::Schedule(
		bbque::SystemView & sv) {
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

	// Release the resource view
	rsrc_acct.PutView(rsrc_view_token);

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

		// Order schedule entities by metrics
		result = OrderSchedEntity(sched_map, sv, prio, cl_id);
		if (result == SCHED_CLUSTER_FULL) {
			clusters_full[cl_id] = true;
			continue;
		}

		// Nothing to schedule in this cluster
		if (sched_map.empty())
			continue;

		if (result != SCHED_OK)
			return result;

		// For each application schedule a working mode
		SelectWorkingModes(sched_map);
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
	logger->Debug(
			"____________________| Scheduling entities |____________________");

	// The scheduling entities should be picked in a descending order of
	// metrics value
	SchedEntityMap_t::reverse_iterator se_it(sched_map.rbegin());
	SchedEntityMap_t::reverse_iterator end_se(sched_map.rend());
	float eval_metrics;

	// Pick the entity and set the new Application Working Mode
	for (; se_it != end_se; ++se_it) {
		AppPtr_t & papp = (se_it->second).first;
		AwmPtr_t & eval_awm = (se_it->second).second;

		// Check a set of conditions accordingly to skip current
		// application/EXC
		if (CheckSkipConditions(papp))
			continue;

		// Get the metrics of the AWM to schedule
		eval_metrics = *(static_cast<float *>
				(eval_awm->GetAttribute(SCHEDULER_POLICY_NAME,
										"metrics").get()));

		logger->Debug("Selecting: [%s] schedule request for AWM{%d}...",
				papp->StrId(),
				eval_awm->Id());

		// Get the resource binding of the current AWM
		UsagesMapPtr_t rsrc_usages =
			UsagesMapPtr_t(static_cast<UsagesMap_t *>(eval_awm->GetAttribute(
							SCHEDULER_POLICY_NAME, "binding").get()));
		assert(rsrc_usages->size() == eval_awm->ResourceUsages().size());

		// Schedule the application in the working mode just evaluated
		Application::ExitCode_t result =
			papp->ScheduleRequest(eval_awm, rsrc_usages, rsrc_view_token);

		// Debugging messages
		if (result != Application::APP_WM_ACCEPTED) {
			logger->Debug("Selecting: [%s] AWM{%d} rejected ! [ret %d]",
							papp->StrId(),
							eval_awm->Id(),
							result);
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


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::InsertWorkingModes(
		SchedEntityMap_t & sched_map,
		AppPtr_t const & papp,
		int cl_id) {
	// Working modes
	AwmPtrList_t const * awms = papp->WorkingModes();
	AwmPtrList_t::const_iterator awm_it(awms->begin());
	AwmPtrList_t::const_iterator end_awm(awms->end());
	for (; awm_it != end_awm; ++awm_it) {
		logger->Debug("Insert: [%s] AWM{%d} metrics computing...", papp->StrId(),
				(*awm_it)->Id());

		// Skip if the application has been disabled/stopped in the meanwhile
		if (papp->Disabled()) {
			logger->Debug("Insert: [%s] disabled/stopped during scheduling [Ord]",
					papp->StrId());
			return SCHED_SKIP_APP;
		}

		// Metrics computation
		float * metrics = new float;
		ExitCode_t result =
			MetricsComputation(papp, (*awm_it), cl_id, *metrics);

		switch (result) {
		case SCHED_CLUSTER_FULL:
			logger->Warn("Insert: No more PEs in cluster %d", cl_id);
			return result;

		case SCHED_RSRC_UNAV:
			logger->Warn("Insert: [%s] AWM{%d} CL=%d unavailable resources "
					"[RA:%d]", papp->StrId(), (*awm_it)->Id(), cl_id, result);
			continue;

		case SCHED_ERROR:
			logger->Error("Insert: An error occurred [ret %d]", result);
			return result;

		default:
			break;
		}

		// Set the metrics value and insert the entity into the map
		(*awm_it)->SetAttribute(
				SCHEDULER_POLICY_NAME, "metrics", VoidPtr_t(metrics));
		sched_map.insert(std::pair<float, SchedEntity_t>(
					static_cast<float>(*metrics),
					SchedEntity_t(papp, (*awm_it))));

		logger->Info("Insert: [%s] AWM{%d} CL=%d metrics %.4f", papp->StrId(),
				(*awm_it)->Id(), cl_id, *metrics);
	}

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

	// If the resource binding implies migration from a cluster to another we
	// have to evaluate the overheads
	float migr_cost = GetMigrationOverhead(papp, wm, cl_id);

	// If the working mode is different from the current one, the Execution
	// Context should be reconfigured. Let's estimate the overhead.
	float reconf_cost = GetReconfigOverhead(papp, wm);

	// Contention level
	float cont_level;
	result = GetContentionLevel(wm, cl_id, cont_level);
	if (result != SCHED_OK)
		return result;

	// Metrics
	logger->Debug("AWM value: %.2f", wm->Value());
	metrics = (wm->Value() - reconf_cost - migr_cost) / cont_level;

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

	if (papp->CurrentAWM() && (papp->CurrentAWM() != wm)) {
		OverheadPtr_t reconf_over(
				papp->CurrentAWM()->OverheadInfo(wm->Id()));
		if (reconf_over){

//			logger->Debug("Reconfiguration overhead : %.4f",
//					reconf_over->MaxTime());
			return reconf_over->MaxTime();
		}
	}

//	logger->Debug("Reconfiguration overhead : 0.0");
	return 0.0;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::GetContentionLevel(
		AwmPtr_t const & wm,
		int cl_id,
		float & cont_level) {

	// Safety data check
	if (!wm) {
		logger->Crit("Contention level: Missing working mode.\n"
				"Possibile data corruption in %s",
				SCHEDULER_POLICY_NAMESPACE"yamca");
		assert(!wm);
		return SCHED_ERROR;
	}

	// Bind the clustered resources request into the current cluster.
	// Note: The current policy doesn't support multi-cluster resources
	logger->Debug("Contention level: Binding into cluster %d", cl_id);
	UsagesMapPtr_t rsrc_usages = UsagesMapPtr_t(new UsagesMap_t());
	WorkingMode::ExitCode_t result =
		wm->BindResource("cluster", RSRC_ID_ANY, cl_id, rsrc_usages);

	if (result == WorkingMode::WM_RSRC_MISS_BIND)
		logger->Error("Contention level: {AWM %d} [cluster = %d]"
				"Incomplete resources binding. %d / %d resources bound.",
						wm->Id(),cl_id,	rsrc_usages->size(),
						wm->ResourceUsages().size());

	// Keep track of the resource binding map we want to set
	wm->SetAttribute(SCHEDULER_POLICY_NAME, "binding", VoidPtr_t(rsrc_usages));

	// Contention level
	return ComputeContentionLevel(rsrc_usages, cont_level);
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::ComputeContentionLevel(
		UsagesMapPtr_t const & rsrc_usages,
		float & cont_level) {

	uint64_t rsrc_avail;
	cont_level = 0;

	// Check the availability of the resources requested
	UsagesMap_t::const_iterator usage_it(rsrc_usages->begin());
	UsagesMap_t::const_iterator end_usage(rsrc_usages->end());
	while (usage_it != end_usage) {

		// Query resource availability
		rsrc_avail = rsrc_acct.Available(usage_it->second, rsrc_view_token);
		logger->Debug("{%s} availability = %llu", usage_it->first.c_str(),
				rsrc_avail);

		// If all the PEs have been booked return
		if ((rsrc_avail == 0) &&
				(PathTemplate(usage_it->first).compare(RSRC_CLUST_PE) == 0))
			return SCHED_CLUSTER_FULL;

		// If there is not enough resource return
		if (rsrc_avail < usage_it->second->value) {
			logger->Debug("Contention level: [%s] R=%d / A=%d",
					usage_it->first.c_str(),
					usage_it->second->value,
					rsrc_avail);

			return SCHED_RSRC_UNAV;
		}

		// Update the contention level (inverse)
		cont_level += ((float) usage_it->second->value) / (float) rsrc_avail;
		++usage_it;
	}

	// Avoid division by zero (in the caller)
	if (cont_level == 0)
		++cont_level;

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

