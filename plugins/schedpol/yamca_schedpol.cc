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
	rsrc_acct.PutView(rsrc_view_token);
}


//----- Scheduler policy module interface

char const * YamcaSchedPol::Name() {
	return SCHEDULER_POLICY_NAME;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::Schedule(
		bbque::SystemView const & system) {
	logger->Debug("<<<<<<<<<<<<<<<< Scheduling policy starting...");

	// Get a resources view from Resource Accounter
	InitResourceView();

	// Get the number of clusters
	num_clusters = system.ResourceTotal(RSRC_CLUSTER);
	logger->Debug("There are %d clusters on the platform.", num_clusters);

	// Order scheduling entities from READY and RUNNING applications
	for (uint16_t cl_id = 0; cl_id < num_clusters; ++cl_id) {
		SchedEntityMap_t sched_map;
		logger->Debug("----------------------------------------------");
		logger->Debug("------------------ Cluster %d -----------------",
				cl_id);
		logger->Debug("----------------------------------------------");

		logger->Debug("Applications RUNNING = %d",
				system.ApplicationsRunning()->size());
		if (OrderSchedEntity(sched_map, system.ApplicationsRunning(), cl_id)
				== SCHED_ERROR)
			return SCHED_ERROR;

		logger->Debug("Applications READY = %d",
				system.ApplicationsReady()->size());
		if (OrderSchedEntity(sched_map, system.ApplicationsReady(), cl_id)
				== SCHED_ERROR)
			return SCHED_ERROR;

		// For each application set a working mode
		if (!sched_map.empty())
			SelectWorkingModes(sched_map);
	}

	logger->Debug(">>>>>>>>>>>>>>> Scheduling policy exiting...");

	/**
	 * TODO:
	 * This call should be removed once the synchronized resource acquisition is
	 * implemeted in ResourceAccounter.
	 * Right now this call is a way to make visible the resource acquisition
	 * that the current scheduling has determined.
	 */
	rsrc_acct.SetAsSystemState(rsrc_view_token);
	return SCHED_DONE;
}


RViewToken_t YamcaSchedPol::InitResourceView() {
	tok_counter == std::numeric_limits<uint32_t>::max() ?
		tok_counter = 0:
		++tok_counter;

	std::stringstream tkss;
	tkss << tok_counter;
	std::string strView(
			SCHEDULER_POLICY_NAMESPACE
			SCHEDULER_POLICY_NAME);
	strView += tkss.str();

	rsrc_view_token = rsrc_acct.GetNewView(strView.c_str());
	logger->Debug("%s", strView.c_str());
	logger->Debug("Resources view token assigned: %d",	rsrc_view_token);
	return rsrc_view_token;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::OrderSchedEntity(
		SchedEntityMap_t & sched_map,
		AppsUidMap_t const * apps,
		int cl_id) {

	// Applications to be scheduled
	AppsMap_t::const_iterator apps_it(apps->begin());
	AppsMap_t::const_iterator end_apps(apps->end());
	for (; apps_it != end_apps; ++apps_it) {

		// Check if the application has been scheduled yet
		assert(apps_it->second);
		if (apps_it->second->NextAWM()) {
			logger->Debug("[%d] ""%s"" scheduled yet on %d",
						(apps_it->second)->Pid(),
						(apps_it->second)->Name().c_str(),
						(apps_it->second)->NextAWM()->Id());
			continue;
		}

		// Compute the metrics for all the working modes
		ExitCode_t result =
			InsertWorkingModes(sched_map, apps_it->second, cl_id);
		if (result == SCHED_ERROR) {
			logger->Error("Ordering: Error [ret %d]", result);
			return result;
		}
	}

	return SCHED_OK;
}


inline void YamcaSchedPol::SelectWorkingModes(SchedEntityMap_t & sched_map) {
	logger->Debug("..... Picking the scheduling entities (App + AWM) .....");

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

		logger->Debug("[%s] schedule request for AWM{%d}...", app->StrId(),
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
			logger->Debug("[%s] AWM{%d} rejected ! [code = %d]",
							app->StrId(),
							eval_awm->Id(),
							result);
			continue;
		}

		assert(app->NextAWM());
		AwmPtr_t const & new_awm = app->NextAWM();
		logger->Info("[%s] set to AWM{%d} on clusters map [%s]",
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
		logger->Debug("[%s] AWM{%d} metrics computing...", papp->StrId(),
				(*awm_it)->Id());

		// Metrics computation
		double * metrics = new double;
		ExitCode_t result =
			MetricsComputation(papp, (*awm_it), cl_id, *metrics);
		switch (result) {
		case SCHED_RSRC_UNAV:
			logger->Debug("Resources unavailables [ret %d]", result);
			continue;
		case SCHED_ERROR:
			logger->Error("An error occurred [ret %d]", result);
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

		logger->Debug("\tmetrics = %4.4f", *metrics);
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
	metrics = ((double) wm->Value() - reconf_cost - migr_cost) / cont_level;
	logger->Debug("\t{ Val = %d Rec = %4.2f Migr = %4.2f ContLv = %4.4f }",
				wm->Value(),
				reconf_cost,
				migr_cost,
				cont_level);

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
	return 0.0;
}


inline double GetReconfigOverhead(AppPtr_t const & papp,
		AwmPtr_t const & wm) {

	if (papp->CurrentAWM() && (papp->CurrentAWM() != wm)) {
		OverheadPtr_t reconf_over(
				papp->CurrentAWM()->OverheadInfo(wm->Id()));
		if (reconf_over)
			return reconf_over->MaxTime();
	}

	return 0.0;
}


SchedulerPolicyIF::ExitCode_t YamcaSchedPol::GetContentionLevel(
		AwmPtr_t const & wm,
		int cl_id,
		double & cont_level) {
	// Safety data check
	if (!wm) {
		logger->Crit("Missing working mode.\n"
				"Error: possibile data corruption in %s",
				SCHEDULER_POLICY_NAMESPACE"yamca");
		assert(!wm);
		return SCHED_ERROR;
	}

	// Bind the clustered resources request into the current cluster.
	// Note: The current policy doesn't support multi-cluster resources
	logger->Debug("\tBinding on cluster %d", cl_id);
	UsagesMapPtr_t rsrc_usages = UsagesMapPtr_t(new UsagesMap_t());
	WorkingMode::ExitCode_t result =
		wm->BindResource("cluster", RSRC_ID_ANY, cl_id, rsrc_usages);

	if (result == WorkingMode::WM_RSRC_MISS_BIND)
		logger->Error("{AWM %d} [cluster = %d] Incomplete resources binding."
						"%d / %d resources bind.",
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
		if (rsrc_avail < usage_it->second->value) {
			logger->Debug("[%s] Requested = %d | Available %d in cluster %d",
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

