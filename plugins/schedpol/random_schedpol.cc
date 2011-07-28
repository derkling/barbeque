/**
 *       @file  random_schedpol.cc
 *      @brief  The Random resource scheduler (dynamic plugin)
 *
 * This implements a dynamic C++ plugin which implements the Random resource
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

#include "random_schedpol.h"

#include "bbque/modules_factory.h"
#include "bbque/system_view.h"
#include "bbque/app/application.h"
#include "bbque/app/working_mode.h"
#include "bbque/plugins/logger.h"
#include "bbque/res/resource_accounter.h"

#include <iostream>
#include <random>

namespace ba = bbque::app;
namespace br = bbque::res;

namespace bbque { namespace plugins {

RandomSchedPol::RandomSchedPol() :
	dist(0, 100) {

	// Get a logger
	plugins::LoggerIF::Configuration conf(
			SCHEDULER_POLICY_NAMESPACE
			SCHEDULER_POLICY_NAME);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		std::cout << "Random: Build random plugin "
			<< this << "] FAILED (Error: missing logger module)" << std::endl;
		assert(logger);
	}

	logger->Info("Random: Built a new dynamic object[%p]\n", this);

}

RandomSchedPol::~RandomSchedPol() {
}

//----- Scheduler policy module interface

char const * RandomSchedPol::Name() {
	return SCHEDULER_POLICY_NAME;
}


/**
 * The RNG used for AWM selection.
 */
std::mt19937 rng_engine(time(0));

void RandomSchedPol::ScheduleApp(AppPtr_t papp) {
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	ba::WorkingMode::ExitCode_t bindResult;
	ba::AwmPtrList_t::const_iterator it;
	ba::AwmPtrList_t::const_iterator end;
	ba::AwmPtrList_t const *awms;
	br::UsagesMapPtr_t pum;
	uint32_t selected_awm;
	uint32_t selected_cluster;
	uint8_t cluster_count;

	assert(papp);

	// Check for a valid cluster count
	cluster_count = ra.Total(RSRC_CLUSTER);
	if (cluster_count == 0) {
		assert(cluster_count != 0);
		return;
	}

	// Select a random AWM for this EXC
	awms = papp->WorkingModes();
	selected_awm = dist(rng_engine) % awms->size();
	logger->Debug("Scheduling EXC [%s] on AWM [%d of %d]",
			papp->StrId(), selected_awm, awms->size());
	it = awms->begin();
	end = awms->end();
	for ( ; selected_awm && it!=end; --selected_awm, ++it);
	assert(it!=end);

	// Bind to a random virtual cluster
	selected_cluster = dist(rng_engine) % cluster_count;
	logger->Debug("Scheduling EXC [%s] on Cluster [%d of %d]",
			papp->StrId(), selected_cluster, ra.Total(RSRC_CLUSTER));
	bindResult = (*it)->BindResource("cluster", RSRC_ID_ANY, selected_cluster, pum);
	if (bindResult != ba::WorkingMode::WM_SUCCESS) {
		logger->Error("Resource biding for EXC [%s] FAILED", papp->StrId());
		return;
	}

	// Schedule the selected AWM on the selected Cluster
	papp->ScheduleRequest((*it), pum, ra_view);

}


SchedulerPolicyIF::ExitCode_t
RandomSchedPol::Schedule(bbque::SystemView & sv) {
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	br::ResourceAccounter::ExitCode_t viewResult;
	AppsUidMapIt app_it;
	AppPtr_t papp;

	if (!logger) {
		assert(logger);
		return SCHED_ERROR;
	}

	// Get a new view on the ResourceAccounter
	viewResult = ra.GetView(SCHEDULER_POLICY_NAMESPACE
			SCHEDULER_POLICY_NAME, ra_view);
	if (viewResult != ResourceAccounter::RA_SUCCESS) {
		logger->Crit("Initialization failed "
				"(Error: unable to get a view from RA)");
		return SCHED_ERROR;
	}

	logger->Info("Random scheduling RUNNING applications...");

	papp = sv.GetFirstRunning(app_it);
	while (papp) {
		ScheduleApp(papp);
		papp = sv.GetNextRunning(app_it);
	}

	logger->Info("Random scheduling READY applications...");

	papp = sv.GetFirstReady(app_it);
	while (papp) {
		ScheduleApp(papp);
		papp = sv.GetNextReady(app_it);
	}

	// Release the ResourceAccounter view
	ra.PutView(ra_view);

	return SCHED_DONE;
}

//----- static plugin interface

void * RandomSchedPol::Create(PF_ObjectParams *) {
	return new RandomSchedPol();
}

int32_t RandomSchedPol::Destroy(void * plugin) {
  if (!plugin)
    return -1;
  delete (RandomSchedPol *)plugin;
  return 0;
}

} // namesapce plugins

} // namespace bque

