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

#include "yams_schedpol.h"

#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <functional>

#include "bbque/cpp11/thread.h"
#include "bbque/modules_factory.h"
#include "bbque/app/working_mode.h"
#include "bbque/plugins/logger.h"
#include "contrib/sched_contrib_manager.h"

namespace bu = bbque::utils;
namespace po = boost::program_options;

namespace bbque { namespace plugins {


SchedContribManager::SCType_t YamsSchedPol::sc_types[] = {
	SchedContribManager::VALUE,
	SchedContribManager::RECONFIG,
	SchedContribManager::CONGESTION,
	SchedContribManager::FAIRNESS
};

// Definition of time metrics of the scheduling policy
MetricsCollector::MetricsCollection_t
YamsSchedPol::coll_metrics[YAMS_METRICS_COUNT] = {
	YAMS_SAMPLE_METRIC("ord",
			"Time to order SchedEntity into a cluster [ms]"),
	YAMS_SAMPLE_METRIC("sel",
			"Time to select AWMs/Clusters for the EXC [ms]"),
	YAMS_SAMPLE_METRIC("mcomp",
			"Time for computing a single metrics [ms]"),
	YAMS_SAMPLE_METRIC("awmvalue",
			"AWM value of the scheduled entity")
};

// Definition of time metrics for each SchedContrib computation
MetricsCollector::MetricsCollection_t
YamsSchedPol::coll_mct_metrics[YAMS_SC_COUNT] = {
	YAMS_SAMPLE_METRIC("awmv.comp",
			"AWM value computing time [ms]"),
	YAMS_SAMPLE_METRIC("recf.comp",
			"Reconfiguration contribute computing time [ms]"),
	YAMS_SAMPLE_METRIC("cgst.comp",
			"Congestion contribute computing time [ms]"),
	YAMS_SAMPLE_METRIC("fair.comp",
			"Fairness contribute computing time [ms]")
	// ...:: ADD_MCT ::...
};

// :::::::::::::::::::::: Static plugin interface ::::::::::::::::::::::::::::

void * YamsSchedPol::Create(PF_ObjectParams *) {
	return new YamsSchedPol();
}

int32_t YamsSchedPol::Destroy(void * plugin) {
	if (!plugin)
		return -1;
	delete (YamsSchedPol *)plugin;
	return 0;
}

// ::::::::::::::::::::: Scheduler policy module interface :::::::::::::::::::

char const * YamsSchedPol::Name() {
	return SCHEDULER_POLICY_NAME;
}

YamsSchedPol::YamsSchedPol():
	cm(ConfigurationManager::GetInstance()),
	ra(ResourceAccounter::GetInstance()),
	mc(bu::MetricsCollector::GetInstance()) {

	// Get a logger
	plugins::LoggerIF::Configuration conf(MODULE_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

	if (logger)
		logger->Info("yams: Built a new dynamic object[%p]", this);
	else
		fprintf(stderr, FI("yams: Built new dynamic object [%p]\n"), (void *)this);

	// Instantiate the SchedContribManager
	scm = new SchedContribManager(sc_types, YAMS_SC_COUNT);

	// Resource view counter
	vtok_count = 0;

	// Register all the metrics to collect
	mc.Register(coll_metrics, YAMS_METRICS_COUNT);
	mc.Register(coll_mct_metrics, YAMS_SC_COUNT);
}

YamsSchedPol::~YamsSchedPol() {

}

YamsSchedPol::ExitCode_t YamsSchedPol::Init() {
	// Set the counter (avoiding overflow)
	vtok_count == std::numeric_limits<uint32_t>::max() ?
		vtok_count = 0:
		++vtok_count;

	// Build a string path for the resource state view
	std::string schedpolname(MODULE_NAMESPACE);
	char token_path[30];
	snprintf(token_path, 30, "%s%d", schedpolname.c_str(), vtok_count);

	// Get a resource state view
	ResourceAccounterStatusIF::ExitCode_t ra_result;
	ra_result = ra.GetView(token_path, vtok);
	if (ra_result != ResourceAccounterStatusIF::RA_SUCCESS) {
		logger->Fatal("Init: Cannot get a resource state view");
		return YAMS_ERR_VIEW;
	}

	logger->Debug("Init: Requiring state view token for %s", token_path);
	logger->Debug("Init: Resources state view token = %d", vtok);

	// Get the number of clusters
	cl_info.rsrcs = sv->GetResources(RSRC_CLUSTER);
	cl_info.num   = cl_info.rsrcs.size();
	cl_info.ids.resize(cl_info.num);
	if (cl_info.num == 0) {
		logger->Error("Init: No clusters available on the platform");
		return YAMS_ERR_CLUSTERS;
	}

	// Get all the clusters IDs
	ResourcePtrList_t::iterator cl_it(cl_info.rsrcs.begin());
	ResourcePtrList_t::iterator end_cl(cl_info.rsrcs.end());
	for (uint8_t j = 0; cl_it != end_cl; ++cl_it, ++j) {
		ResourcePtr_t & rsrc(*cl_it);
		cl_info.ids[j] = ResourcePathUtils::GetID(rsrc->Name(), "cluster");
		logger->Debug("Init: Cluster ID: %d", cl_info.ids[j]);
	}

	logger->Debug("Init: Clusters on the platform: %d", cl_info.num);
	logger->Debug("Init: Lowest application prio : %d",
			sv->ApplicationLowestPriority());

	// Set the view information into the metrics contribute
	scm->SetViewInfo(sv, vtok);

	return YAMS_SUCCESS;
}

SchedulerPolicyIF::ExitCode_t
YamsSchedPol::Schedule(System & sys_if, RViewToken_t & rav) {
	ExitCode_t result;
	logger->Debug("@@@@@@@@@@@@@@@@ Scheduling policy starting @@@@@@@@@@@@");

	// Save a reference to the System interface;
	sv = &sys_if;

	// Initialize a new resources state view
	result = Init();
	if (result != YAMS_SUCCESS)
		goto error;

	// Schedule per priority
	for (AppPrio_t prio = 0; prio <= sv->ApplicationLowestPriority(); ++prio) {
		if (!sv->HasApplications(prio))
			continue;
		SchedulePrioQueue(prio);
	}

	// Set the new resource state view token
	rav = vtok;

	// Cleaning
	entities.clear();
	cl_info.full.reset();

	ra.PrintStatusReport(vtok);
	logger->Debug("################ Scheduling policy exiting ##############");

	return SCHED_DONE;

error:
	logger->Error("Schedule: an error occurred. Interrupted.");
	entities.clear();
	cl_info.full.reset();

	ra.PutView(vtok);
	return SCHED_ERROR;
}

void YamsSchedPol::SchedulePrioQueue(AppPrio_t prio) {
	std::vector<ResID_t>::iterator ids_it;
	std::vector<ResID_t>::iterator end_ids;
	bool sched_incomplete;
	uint8_t naps_count = 0;
	SchedContribPtr_t sc_fair;

	// Reset timer
	YAMS_RESET_TIMING(yams_tmr);

do_schedule:
	// Init fairness contribute
	sc_fair = scm->GetContrib(SchedContribManager::FAIRNESS);
	assert(sc_fair != nullptr);
	sc_fair->Init(&prio);

	// For each cluster/node evaluate...
	ids_it = cl_info.ids.begin();
	for (; ids_it != cl_info.ids.end(); ++ids_it) {
		ResID_t & cl_id(*ids_it);
		logger->Debug("Schedule: :::::::::::::::::::::: Cluster %d:", cl_id);

		// Skip current cluster if full
		if (cl_info.full[cl_id]) {
			logger->Debug("Schedule: cluster %d is full, skipping...", cl_id);
			continue;
		}

		// Order schedule entities by aggregate metrics
		naps_count = OrderSchedEntities(prio, cl_id);
	}
	// Collect "ordering step" metrics
	YAMS_GET_TIMING(coll_metrics, YAMS_ORDERING_TIME, yams_tmr);

	// Selection: for each application schedule a working mode
	YAMS_RESET_TIMING(yams_tmr);
	sched_incomplete = SelectSchedEntities(naps_count);
	entities.clear();

	if (sched_incomplete)
		goto do_schedule;

	// Stop timing metrics
	YAMS_GET_TIMING(coll_metrics, YAMS_SELECTING_TIME, yams_tmr);
}

uint8_t YamsSchedPol::OrderSchedEntities(AppPrio_t prio, uint16_t cl_id) {
	uint8_t naps_count = 0;
	AppsUidMapIt app_it;
	AppCPtr_t papp;

	// Applications to be scheduled
	papp = sv->GetFirstWithPrio(prio, app_it);
	for (; papp; papp = sv->GetNextWithPrio(prio, app_it)) {
		// Check if the Application/EXC must be skipped
		if (CheckSkipConditions(papp))
			continue;

		// Compute the metrics for each AWM binding resources to cluster 'cl_id'
		InsertWorkingModes(papp, cl_id);

		// Keep track of NAPped Applications/EXC
		if (papp->GetGoalGap())
			++naps_count;
	}

	// Order the scheduling entities list
	entities.sort(CompareEntities);

	return naps_count;
}

bool YamsSchedPol::SelectSchedEntities(uint8_t naps_count) {
	Application::ExitCode_t app_result;
	SchedEntityList_t::iterator se_it(entities.begin());
	SchedEntityList_t::iterator end_se(entities.end());
	logger->Debug("=================| Scheduling entities |=================");

	// Pick the entity and set the new AWM
	for (; se_it != end_se; ++se_it) {
		SchedEntityPtr_t & pschd(*se_it);

		// Skip this AWM-Cluster if the cluster is full or if the
		// Application/EXC must be skipped
		if (cl_info.full.test(pschd->clust_id) ||
				(CheckSkipConditions(pschd->papp)))
			continue;

		// Send the schedule request
		app_result = pschd->papp->ScheduleRequest(pschd->pawm, vtok,
				pschd->clust_id);
		logger->Debug("Selecting: [%s] schedule requested", pschd->StrId());

		// Scheduling request rejected
		if (app_result != ApplicationStatusIF::APP_WM_ACCEPTED) {
			logger->Debug("Selecting: [%s] rejected !", pschd->StrId());
			continue;
		}

		// Logging messages
		if (!pschd->papp->Synching() || pschd->papp->Blocking()) {
			logger->Debug("Selecting: [%s] state %s|%s", pschd->papp->StrId(),
					Application::StateStr(pschd->papp->State()),
					Application::SyncStateStr(pschd->papp->SyncState()));
			continue;
		}
		logger->Notice("Selecting: [%s] scheduled << metrics: %.4f >>",
				pschd->StrId(), pschd->metrics);

		// Set the application value (scheduling aggregate metrics)
		pschd->papp->SetValue(pschd->metrics);

		// Sample the AWM value for future evaluation of the scheduling results
		YAMS_GET_SAMPLE(coll_metrics, YAMS_METRICS_AWMVALUE,
				pschd->pawm->Value());

		// Break as soon as all NAPped apps have been scheduled
		if (naps_count && (--naps_count == 0))
			break;
	}

	if (se_it != end_se) {
		logger->Debug("======================| NAP Break |===================");
		return true;
	}

	logger->Debug("========================| DONE |======================");
	return false;
}

void YamsSchedPol::InsertWorkingModes(AppCPtr_t const & papp, uint16_t cl_id) {
	std::list<std::thread> awm_thds;
	float metrics = 0.0;

	// Application Working Modes
	AwmPtrList_t const * awms = papp->WorkingModes();
	AwmPtrList_t::const_iterator awm_it(awms->begin());
	AwmPtrList_t::const_iterator end_awm(awms->end());

	// AWMs (+resources bound to 'cl_id') evaluation
	for (; awm_it != end_awm; ++awm_it) {
		AwmPtr_t const & pawm(*awm_it);
		SchedEntityPtr_t pschd(new SchedEntity_t(papp, pawm, cl_id, metrics));
#ifdef BBQUE_SP_YAMS_PARALLEL
		awm_thds.push_back(
				std::thread(&YamsSchedPol::EvalWorkingMode, this, pschd)
		);
#else
		EvalWorkingMode(pschd);
#endif
	}

#ifdef BBQUE_SP_YAMS_PARALLEL
	for_each(awm_thds.begin(), awm_thds.end(), mem_fn(&std::thread::join));
	awm_thds.clear();
#endif
	logger->Debug("Evaluate: table size = %d", entities.size());
}

void YamsSchedPol::EvalWorkingMode(SchedEntityPtr_t pschd) {
	std::unique_lock<std::mutex> sched_ul(sched_mtx, std::defer_lock);
	ExitCode_t result;
	logger->Debug("Insert: [%s] ...metrics computing...", pschd->StrId());

	// Skip if the application has been disabled/stopped in the meanwhile
	if (pschd->papp->Disabled()) {
		logger->Debug("Insert: [%s] disabled/stopped during schedule ordering",
				pschd->papp->StrId());
		return;
	}

	// Bind the resources of the AWM to the current cluster
	result = BindCluster(pschd);
	if (result != YAMS_SUCCESS)
		return;

	// Metrics computation
	Timer comp_tmr;
	YAMS_RESET_TIMING(comp_tmr);
	AggregateContributes(pschd);
	YAMS_GET_TIMING(coll_metrics, YAMS_METRICS_COMP_TIME, comp_tmr);

	// Insert the SchedEntity in the scheduling list
	sched_ul.lock();
	entities.push_back(pschd);
	logger->Debug("Insert [%d]: %s: ..:: metrics %1.3f",
			entities.size(), pschd->StrId(), pschd->metrics);
}

void YamsSchedPol::AggregateContributes(SchedEntityPtr_t pschd) {
	SchedContribManager::ExitCode_t scm_ret;
	SchedContrib::ExitCode_t sc_ret;
	char metrics_log[255];
	uint8_t len = 0;
	float sc_value;

	for (int i = 0; i < YAMS_SC_COUNT; ++i) {
		// Timer
		Timer comp_tmr;

		sc_value = 0.0;
		EvalEntity_t const & eval_ent(*pschd.get());
		YAMS_RESET_TIMING(comp_tmr);

		// Compute the single contribution
		scm_ret = scm->GetIndex(sc_types[i], eval_ent, sc_value, sc_ret);
		if (scm_ret != SchedContribManager::OK) {

			logger->Error("Aggregate: [SchedContribManager error %d]", scm_ret);
			if (scm_ret != SchedContribManager::SC_ERROR) {
				YAMS_RESET_TIMING(comp_tmr);
				continue;
			}

			// SchedContrib specific error handling
			switch (sc_ret) {
			case SchedContrib::MCT_RSRC_NO_PE:
				logger->Debug("Aggregate: No available PEs in cluster/node %d",
						pschd->clust_id);
				cl_info.full.set(pschd->clust_id);
				return;
			default:
				logger->Warn("Aggregate: Unable to schedule into cluster/node %d"
						" [SchedContrib error %d]", pschd->clust_id);
				YAMS_GET_TIMING(coll_mct_metrics, i, comp_tmr);
				continue;
			}
		}
		YAMS_GET_TIMING(coll_mct_metrics, i, comp_tmr);

		// Cumulate the contribution
		pschd->metrics += sc_value;
		len += sprintf(metrics_log+len, "%c: %5.4f, ",
				scm->GetString(sc_types[i])[0],
				sc_value);
	}

	metrics_log[len-2] = '\0';
	logger->Notice("Aggregate: %s app-value: (%s) => %5.4f", pschd->StrId(),
			metrics_log, pschd->metrics);
}

YamsSchedPol::ExitCode_t YamsSchedPol::BindCluster(SchedEntityPtr_t pschd) {
	WorkingModeStatusIF::ExitCode_t awm_result;
	AwmPtr_t & pawm(pschd->pawm);
	ResID_t & cl_id(pschd->clust_id);

	// Binding of the AWM resource into the current cluster.
	// The cluster ID is also used as reference for the resource binding,
	// since the policy handles more than one binding per AWM.
	awm_result = pawm->BindResource("cluster", RSRC_ID_ANY, cl_id, cl_id);

	// The cluster binding should never fail
	if (awm_result == WorkingModeStatusIF::WM_RSRC_MISS_BIND) {
		logger->Error("BindCluster: {AWM %d} [cluster %d]"
				"Incomplete	resources binding. %d / %d resources bound.",
				pawm->Id(), cl_id,
				pawm->GetSchedResourceBinding()->size(),
				pawm->RecipeResourceUsages().size());
		assert(awm_result == WorkingModeStatusIF::WM_SUCCESS);
		return YAMS_ERROR;
	}
	logger->Debug("BindCluster: {AWM %d} resources bound to cluster %d",
			pawm->Id(), cl_id);

	return YAMS_SUCCESS;
}

bool YamsSchedPol::CompareEntities(SchedEntityPtr_t & se1,
		SchedEntityPtr_t & se2) {
	// Metrics (primary sorting key)
	if (se1->metrics < se2->metrics)
		return false;
	if (se1->metrics > se2->metrics)
		return true;

	// Apps asserting a NAP should be considered first
	if ((se1->papp->GetGoalGap() > 0) && (se2->papp->GetGoalGap() == 0))
		return true;
	if ((se1->papp->GetGoalGap() == 0) && (se2->papp->GetGoalGap() > 0))
		return false;

	// Higher value AWM first
	if (se1->pawm->Value() > se2->pawm->Value())
		return true;

	return false;
}


} // namespace plugins

} // namespace bbque
