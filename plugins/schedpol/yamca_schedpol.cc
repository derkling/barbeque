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
#include <sstream>

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
		bbque::SystemView const & system) {
	logger->Debug("<<<<<<<<<<<<<<<< Scheduling policy starting...");

	// Get a resources view from Resource Accounter
	if (InitResourceView() == SCHED_ERROR) {
		logger->Fatal("Schedule: Aborted due to resource state view missing");
		return SCHED_ERROR;
	}

	// Get the number of clusters
	num_clusters = system.ResourceTotal(RSRC_CLUSTER);
	logger->Debug("Schedule: Found %d clusters on the platform.", num_clusters);

	// Order scheduling entities from READY and RUNNING applications
	for (uint16_t cl_id = 0; cl_id < num_clusters; ++cl_id) {
		SchedEntityMap_t sched_map;
		logger->Debug("Schedule: ------------- Cluster %d ---------------",
				cl_id);

		logger->Debug("Schedule: --- %d ExC in the RUNNING queue --------",
				system.ApplicationsRunning()->size());
		if (OrderSchedEntity(sched_map, system.ApplicationsRunning(), cl_id)
				== SCHED_ERROR)
			return SCHED_ERROR;

		logger->Debug("Schedule: --- %d ExC in the READY queue ----------",
				system.ApplicationsReady()->size());
		if (OrderSchedEntity(sched_map, system.ApplicationsReady(), cl_id)
				== SCHED_ERROR)
			return SCHED_ERROR;

		// For each application set a working mode
		if (!sched_map.empty())
			SelectWorkingModes(sched_map);
	}

	logger->Debug(">>>>>>>>>>>>>>> Scheduling policy exiting...");

	// Release the resource view
	rsrc_acct.PutView(rsrc_view_token);
	return SCHED_DONE;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::InitResourceView() {
	tok_counter == std::numeric_limits<uint32_t>::max() ?
		tok_counter = 0:
		++tok_counter;

	std::stringstream tkss;
	tkss << tok_counter;
	std::string strView(
			SCHEDULER_POLICY_NAMESPACE
			SCHEDULER_POLICY_NAME);
	strView += tkss.str();

	ResourceAccounter::ExitCode_t view_result;
	view_result = rsrc_acct.GetView(strView.c_str(), rsrc_view_token);
	if (view_result != ResourceAccounter::RA_SUCCESS) {
		logger->Fatal("Init: Cannot get a resource state view");
		return SCHED_ERROR;
	}

	logger->Debug("Init: Requiring view token for %s", strView.c_str());
	logger->Debug("Init: Resources state view token = %d", rsrc_view_token);
	return SCHED_OK;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::OrderSchedEntity(
		SchedEntityMap_t & sched_map,
		AppsUidMap_t const * apps,
		int cl_id) {

	// Applications to be scheduled
	AppsMap_t::const_iterator apps_it(apps->begin());
	AppsMap_t::const_iterator end_apps(apps->end());
	for (; apps_it != end_apps; ++apps_it) {
		AppPtr_t const & papp = apps_it->second;

		// Skip if the application has been scheduled yet
		assert(papp);
		if (papp->NextAWM()) {
			logger->Debug("Ordering: [%d] scheduled yet into AWM{%d}",
						papp->StrId(),
						papp->NextAWM()->Id());
			continue;
		}

		// Compute the metrics for all the working modes. If the application
		// is disabled/stopped in the meanwhile, it must be skipped.
		ExitCode_t result = InsertWorkingModes(sched_map, papp, cl_id);
		if (result == SCHED_SKIP_APP)
			continue;
		if (result == SCHED_ERROR) {
			logger->Error("Ordering: Error [ret %d]", result);
			return result;
		}
	}

	return SCHED_OK;
}


inline void YamcaSchedPol::SelectWorkingModes(SchedEntityMap_t & sched_map) {
	logger->Debug(".....: Picking scheduling entities (App + AWM) :.....");

	// The scheduling entities should be picked in a descending order of
	// metrics value
	SchedEntityMap_t::reverse_iterator se_it(sched_map.rbegin());
	SchedEntityMap_t::reverse_iterator end_se(sched_map.rend());
	double curr_metrics, eval_metrics;

	// Pick the entity and set the new Application Working Mode
	for (; se_it != end_se; ++se_it) {
		AppPtr_t & app = (se_it->second).first;
		AwmPtr_t const & curr_awm = (se_it->second).first->NextAWM();
		AwmPtr_t & eval_awm = (se_it->second).second;

		// Skip if the application has been disabled or stopped in the
		// meanwhile
		if ((app->State() == Application::DISABLED) ||
				(app->State() == Application::FINISHED)) {
			logger->Debug("Selecting: [%s] disabled/stopped during"
					"scheduling",
					app->StrId());
			continue;
		}

		// Get the metrics of the AWM to schedule
		eval_metrics = *(static_cast<double *>
				(eval_awm->GetAttribute(SCHEDULER_POLICY_NAME,
										"metrics").get()));

		// If an AWM has been previously set it should have a greater metrics.
		// Thus we can skip to the next scheduling entity
		if (curr_awm) {
			curr_metrics = *(static_cast<double *>
					(curr_awm->GetAttribute(SCHEDULER_POLICY_NAME,
											"metrics").get()));
			assert(eval_metrics <= curr_metrics);
			continue;
		}

		logger->Debug("Selecting: [%s] schedule request for AWM{%d}...",
				app->StrId(),
				eval_awm->Id());

		// Get the resource binding of the current AWM
		UsagesMapPtr_t rsrc_usages =
			UsagesMapPtr_t(static_cast<UsagesMap_t *>(eval_awm->GetAttribute(
							SCHEDULER_POLICY_NAME, "binding").get()));
		assert(rsrc_usages->size() == eval_awm->ResourceUsages().size());

		// Schedule the application in the working mode just evaluated
		Application::ExitCode_t result =
			app->ScheduleRequest(eval_awm, rsrc_usages, rsrc_view_token);

		// Debugging messages
		if (result != Application::APP_WM_ACCEPTED) {
			logger->Debug("Selecting: [%s] AWM{%d} rejected ! [ret %d]",
							app->StrId(),
							eval_awm->Id(),
							result);
			continue;
		}

		assert(app->NextAWM());
		AwmPtr_t const & new_awm = app->NextAWM();
		logger->Info("Selecting: [%s] set to AWM{%d} on clusters map [%s]",
					app->StrId(),
					new_awm->Id(),
					new_awm->GetClusterSet().to_string().c_str());
	}
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

		// Skip if the application has been disabled or stopped in the
		// meanwhile
		if ((papp->State() == Application::DISABLED) ||
				(papp->State() == Application::FINISHED)) {
			logger->Debug("Insert: [%s] disabled/stopped during scheduling [Ord]",
					papp->StrId());
			return SCHED_SKIP_APP;
		}

		// Metrics computation
		double * metrics = new double;
		ExitCode_t result =
			MetricsComputation(papp, (*awm_it), cl_id, *metrics);
		switch (result) {
		case SCHED_RSRC_UNAV:
			logger->Debug("Insert: Resources unavailables [ret %d]", result);
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
		sched_map.insert(std::pair<double, SchedEntity_t>(
					static_cast<double>(*metrics),
					SchedEntity_t(papp, (*awm_it))));

		logger->Debug("Metrics = %.4f", *metrics);
	}

	return SCHED_OK;
}

// Functions used by MetricsComputation() to get the migration and
// reconfiguration overhead
double GetMigrationOverhead(AppPtr_t const & papp, AwmPtr_t const & wm,
		int cl_id);
double GetReconfigOverhead(AppPtr_t const & papp, AwmPtr_t const & wm);


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::MetricsComputation(
		AppPtr_t const & papp,
		AwmPtr_t const & wm,
		int cl_id,
		double & metrics) {
	ExitCode_t result;
	metrics = 0.0;

	// If the resource binding implies migration from a cluster to another we
	// have to evaluate the overheads
	double migr_cost = GetMigrationOverhead(papp, wm, cl_id);

	// If the working mode is different from the current one, the Execution
	// Context should be reconfigured. Let's estimate the overhead.
	double reconf_cost = GetReconfigOverhead(papp, wm);

	// Contention level
	double cont_level;
	result = GetContentionLevel(wm, cl_id, cont_level);
	if (result != SCHED_OK)
		return result;

	// Metrics
	logger->Debug("AWM value: %d", wm->Value());
	metrics = ((double) wm->Value() - reconf_cost - migr_cost) / cont_level;

	return SCHED_OK;
}


inline double GetMigrationOverhead(AppPtr_t const & papp,
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


inline double GetReconfigOverhead(AppPtr_t const & papp,
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
		double & cont_level) {
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
	cont_level = 0;

	// Check the availability of the resources requested
	uint64_t rsrc_avail;
	UsagesMap_t::const_iterator usage_it(rsrc_usages->begin());
	UsagesMap_t::const_iterator end_usage(rsrc_usages->end());
	while (usage_it != end_usage) {

		// Query resource availability
		rsrc_avail = rsrc_acct.Available(usage_it->second, rsrc_view_token);
		logger->Debug("{%s} availability = %llu", usage_it->first.c_str(),
				rsrc_avail);

		if (rsrc_avail < usage_it->second->value) {
			logger->Debug("Contention level: [%s] R=%d / A=%d (cluster %d)",
					usage_it->first.c_str(),
					usage_it->second->value,
					rsrc_avail,
					cl_id);

			return SCHED_RSRC_UNAV;
		}

		// Update the contention level (inverse)
		cont_level += ((double) usage_it->second->value) / (double) rsrc_avail;
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

