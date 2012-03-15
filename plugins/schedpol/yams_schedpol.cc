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
#include <thread>

#include "bbque/modules_factory.h"
#include "bbque/app/working_mode.h"
#include "bbque/plugins/logger.h"

#include "contrib/mct_value.h"
#include "contrib/mct_reconfig.h"
#include "contrib/mct_congestion.h"
#include "contrib/mct_fairness.h"
// ...:: ADD_MCT ::...


namespace bu = bbque::utils;
namespace po = boost::program_options;

namespace bbque { namespace plugins {


char const * YamsSchedPol::mct_str[YAMS_MCT_COUNT] = {
	"awmvalue",
	"reconfig",
	"congestion",
	"fairness"
	//"power",
	//"thermal",
	//"stability",
	//"robustness"
	// ...:: ADD_MCT ::...
};

/* Definition of time metrics of the scheduling policy */
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

/* Definition of time metrics per contribute */
MetricsCollector::MetricsCollection_t
YamsSchedPol::coll_mct_metrics[YAMS_MCT_COUNT] = {
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
uint16_t YamsSchedPol::mct_weights[YAMS_MCT_COUNT] = {0};
uint16_t YamsSchedPol::mct_cfg_params[MetricsContribute::MCT_CPT_COUNT]= {0};

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
	char conf_opts[YAMS_MCT_COUNT+MetricsContribute::MCT_CPT_COUNT][40];

	// Get a logger
	plugins::LoggerIF::Configuration conf(
			SCHEDULER_POLICY_NAMESPACE SCHEDULER_POLICY_NAME);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

	if (logger)
		logger->Info("yams: Built a new dynamic object[%p]\n", this);
	else
		std::cout << "yams: Built new dynamic object ["
			<< this << "]" << std::endl;

	// Load the weights of the metrics contributes
	po::options_description opts_desc("Metrics contributes parameters");
	for (int i = 0; i < YAMS_MCT_COUNT; ++i) {
		snprintf(conf_opts[i], 40, "MetricsContribute.%s.weight", mct_str[i]);
		opts_desc.add_options()
			(conf_opts[i],
			 po::value<uint16_t> (&mct_weights[i])->default_value(0),
			"Single metrics contribute weight");
		;
	}

	// Global MetricsContribute config parameters
	for (int i = 0; i < MetricsContribute::MCT_CPT_COUNT; ++i) {
		snprintf(conf_opts[YAMS_MCT_COUNT+i], 40, "MetricsContribute.%s",
				MetricsContribute::ConfigParamsStr[i]);
		opts_desc.add_options()
			(conf_opts[YAMS_MCT_COUNT+i],
			 po::value<uint16_t>
			 (&mct_cfg_params[i])->default_value(
				 MetricsContribute::ConfigParamsDefault[i]),
			"MCT global parameters");
		;
	}

	po::variables_map opts_vm;
	cm.ParseConfigurationFile(opts_desc, opts_vm);

	// Boundaries enforcement (0 <= MSL <= 100)
	for (int i = 0; i < MetricsContribute::MCT_CPT_COUNT; ++i) {
		logger->Debug("%s = \t%d", MetricsContribute::ConfigParamsStr[i],
				mct_cfg_params[i]);
		if (mct_cfg_params[i] > 100) {
			logger->Warn("Parameter %s out of range [0,100]: "
					"found %d. Setting to %d",
					MetricsContribute::ConfigParamsStr[i],
					mct_cfg_params[i],
					MetricsContribute::ConfigParamsDefault[i]);
			mct_cfg_params[i] = MetricsContribute::ConfigParamsDefault[i];
		}
	}

	// Normalize the weights
	NormalizeMCTWeights();
	for (int i = 0; i < YAMS_MCT_COUNT; ++i)
		logger->Debug("MetricsContribute.%s.weight \t= \t%.3f",
			mct_str[i],	mct_weights_norm[i]);

	// Init the vector of contributes
	mcts[YAMS_VALUE] =
		MetricsContribPtr_t(new MCTValue(mct_str[YAMS_VALUE], mct_cfg_params));
	mcts[YAMS_RECONFIG] =
		MetricsContribPtr_t(new	MCTReconfig(mct_str[YAMS_RECONFIG],
					mct_cfg_params));
	mcts[YAMS_CONGESTION] =
		MetricsContribPtr_t(new MCTCongestion(mct_str[YAMS_CONGESTION],
					mct_cfg_params));
	mcts[YAMS_FAIRNESS] =
		MetricsContribPtr_t(new MCTFairness(mct_str[YAMS_FAIRNESS],
					mct_cfg_params));
	// ...:: ADD_MCT ::...

	// Resource view counter
	vtok_count = 0;

	// Register all the metrics to collect
	mc.Register(coll_metrics, YAMS_METRICS_COUNT);
	mc.Register(coll_mct_metrics, YAMS_MCT_COUNT);
}

YamsSchedPol::~YamsSchedPol() {

}

YamsSchedPol::ExitCode_t YamsSchedPol::Init() {
	// Set the counter (avoiding overflow)
	vtok_count == std::numeric_limits<uint32_t>::max() ?
		vtok_count = 0:
		++vtok_count;

	// Build a string path for the resource state view
	std::string schedpolname(
			SCHEDULER_POLICY_NAMESPACE
			SCHEDULER_POLICY_NAME);
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
	cl_info.num = cl_info.rsrcs.size();
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
	for (int i = 0; i < YAMS_MCT_COUNT; ++i) {
		if (!mcts[i])
			continue;
		mcts[i]->SetViewInfo(sv, vtok);
	}

	return YAMS_SUCCESS;
}

void YamsSchedPol::NormalizeMCTWeights() {
	uint16_t sum = 0;

	// Get the highest weight
	for (int j = 0; j < YAMS_MCT_COUNT; ++j)
		sum += mct_weights[j];

	// Normalize
	for (int i = 0; i < YAMS_MCT_COUNT; ++i)
		mct_weights_norm[i] = mct_weights[i] / (float) sum;
}

SchedulerPolicyIF::ExitCode_t
YamsSchedPol::Schedule(SystemView & sview, RViewToken_t & rav) {
	ExitCode_t result;
	logger->Debug("@@@@@@@@@@@@@@@@ Scheduling policy starting @@@@@@@@@@@@");

	// Save a poiner to the SystemView instance;
	sv = &sview;

	// Get a resources view from Resource Accounter
	result = Init();
	if (result != YAMS_SUCCESS)
		goto error;

	// Best-effort apps/EXC: highest to the lowest priority queue iteration
	for (AppPrio_t prio = 0; prio <= sv->ApplicationLowestPriority(); ++prio) {
		// Skip to next priority level there are no applications/EXC
		if (!sv->HasApplications(prio))
			continue;

		// Schedule applications with priority == prio
		SchedulePrioQueue(prio);
	}

	// Set the used resource state view token
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
	// Order scheduling entities
	std::vector<ResID_t>::iterator ids_it(cl_info.ids.begin());
	std::vector<ResID_t>::iterator end_ids(cl_info.ids.end());

	// Reset timer
	YAMS_RESET_TIMING(yams_tmr);

	// Init fairness contribute
	mcts[YAMS_FAIRNESS]->Init(&prio);

	for (; ids_it != end_ids; ++ids_it) {
		ResID_t & cl_id(*ids_it);
		logger->Debug("Schedule: :::::::::::::::::::::: Cluster %d:", cl_id);

		// Skip current cluster if full
		if (cl_info.full[cl_id]) {
			logger->Warn("Schedule: cluster %d is full, skipping...", cl_id);
			continue;
		}

		// Order schedule entities by metrics
		OrderSchedEntities(prio, cl_id);
	}

	// Collect "ordering step" metrics
	YAMS_GET_TIMING(coll_metrics, YAMS_ORDERING_TIME, yams_tmr);
	YAMS_RESET_TIMING(yams_tmr);

	// Selection: for each application schedule a working mode
	SelectSchedEntities();
	entities.clear();

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
		// Check a set of conditions accordingly to skip current
		// Application/EXC
		if (CheckSkipConditions(papp))
			continue;

		// Compute the metrics for all the working modes, binding the
		// resources to the current cluster (cl_id)
		InsertWorkingModes(papp, cl_id);

		// Keep track of NAPped apps
		if (papp->GetGoalGap())
			++naps_count;
	}

	// Order the scheduling entities list
	entities.sort(CompareEntities);

	return naps_count;
}

bool YamsSchedPol::SelectSchedEntities(uint8_t naps_count) {
	Application::ExitCode_t app_result;

	logger->Debug("=================| Scheduling entities |=================");

	// The scheduling entities should be picked in a descending order of
	// metrics value
	SchedEntityList_t::iterator se_it(entities.begin());
	SchedEntityList_t::iterator end_se(entities.end());

	// Pick the entity and set the new Application Working Mode
	for (; se_it != end_se; ++se_it) {
		// The best candidate to schedule
		SchedEntityPtr_t & pschd(*se_it);

		// Skip this AWM-Cluster if the cluster is full
		if (cl_info.full.test(pschd->clust_id))
			continue;

		// Check a set of conditions according to skip current
		// Application/EXC
		if (CheckSkipConditions(pschd->papp))
			continue;

		logger->Debug("Selecting: %s schedule requested", pschd->StrId());

		// Schedule the application in the working mode just evaluated,
		// specifying the binding related to the tracked cluster
		app_result = pschd->papp->ScheduleRequest(pschd->pawm, vtok,
				pschd->clust_id);

		// Scheduling request rejected
		if (app_result != ApplicationStatusIF::APP_WM_ACCEPTED) {
			logger->Debug("Selecting: %s rejected !", pschd->StrId());
			continue;
		}

		// Logging messages
		if (!pschd->papp->Synching() || pschd->papp->Blocking()) {
			logger->Debug("Selecting: [%s] in %s/%s", pschd->papp->StrId(),
					Application::StateStr(pschd->papp->State()),
					Application::SyncStateStr(pschd->papp->SyncState()));
			continue;
		}

		logger->Notice("Selecting: scheduled %s [metrics: %.4f]",
				pschd->StrId(), pschd->metrics);

		// Set the application value (scheduling metrics)
		pschd->papp->SetValue(pschd->metrics);

		// Sample the AWM value of the scheduled application for evaluation of
		// the scheduling results
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


void join_thread(std::thread & t) {
	t.join();
}

void YamsSchedPol::InsertWorkingModes(AppCPtr_t const & papp, uint16_t cl_id) {
	std::list<std::thread> awm_thds;
	float metrics = 0.0;

	// Working modes
	AwmPtrList_t const * awms = papp->WorkingModes();
	AwmPtrList_t::const_iterator awm_it(awms->begin());
	AwmPtrList_t::const_iterator end_awm(awms->end());

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
	for_each(awm_thds.begin(), awm_thds.end(), join_thread);
	awm_thds.clear();
#endif
	logger->Debug("Schedule table size = %d", entities.size());
}

void YamsSchedPol::EvalWorkingMode(SchedEntityPtr_t pschd) {
	ExitCode_t result;
	std::unique_lock<std::mutex> sched_ul(sched_mtx, std::defer_lock);
	logger->Debug("Insert: %s: ...metrics computing...", pschd->StrId());

	// Skip if the application has been disabled/stopped in the meanwhile
	if (pschd->papp->Disabled()) {
		logger->Debug("Insert: {%s} disabled/stopped during schedule ordering",
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
	MetricsContribute::ExitCode_t mct_result;
	float ctrb = 0.0;
	char metrics_log[255];
	uint8_t len = 0;

	for (int i = 0; i < YAMS_MCT_COUNT; ++i) {
		// If weight == 0 or the metrics contribute instance is missing
		// skip the contribute computation
		if (mct_weights_norm[i] == 0 || !mcts[i])
			continue;

		// Timer
		Timer comp_tmr;

		// Compute the single contribute
		EvalEntity_t const & eval_ent(*pschd.get());
		YAMS_RESET_TIMING(comp_tmr);
		mct_result = mcts[i]->Compute(eval_ent, ctrb);
		if (mct_result == MetricsContribute::MCT_RSRC_NO_PE) {
			logger->Debug("Insert: No more PEs in cluster %d",
					pschd->clust_id);
			cl_info.full.set(pschd->clust_id);
			return;
		}
		YAMS_GET_TIMING(coll_mct_metrics, i, comp_tmr);

		// Cumulate the contribute
		len += sprintf(metrics_log+len,  "%c: %5.4f, ", mct_str[i][0],
				mct_weights_norm[i] * ctrb);
		pschd->metrics += mct_weights_norm[i] * ctrb;
	}

	metrics_log[len-2] = '\0';
	logger->Notice("%s App_value: (%s) => %5.4f",
			pschd->StrId(), metrics_log, pschd->metrics);
}

YamsSchedPol::ExitCode_t YamsSchedPol::BindCluster(SchedEntityPtr_t pschd) {
	WorkingModeStatusIF::ExitCode_t awm_result;
	AwmPtr_t & pawm(pschd->pawm);
	ResID_t & cl_id(pschd->clust_id);

	// Binding of the resources requested by the working mode into the current
	// cluster. The cluster ID is used as reference for the resource binding,
	// since the policy handles more than one binding per AWM.
	awm_result = pawm->BindResource("cluster", RSRC_ID_ANY, cl_id, cl_id);

	// The cluster binding should never fail
	if (awm_result == WorkingModeStatusIF::WM_RSRC_MISS_BIND) {
		logger->Error("BindCluster: {AWM %d} [cluster = %d]"
				"Incomplete	resources binding. %d / %d resources bound.",
				pawm->Id(), cl_id, pawm->GetSchedResourceBinding()->size(),
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
	if (se1->metrics <= se2->metrics)
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
