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

#include "random_schedpol.h"

#include "bbque/modules_factory.h"
#include "bbque/system.h"
#include "bbque/app/application.h"
#include "bbque/app/working_mode.h"
#include "bbque/plugins/logger.h"
#include "bbque/resource_accounter.h"

#include <iostream>

namespace ba = bbque::app;
namespace br = bbque::res;

namespace bbque { namespace plugins {

RandomSchedPol::RandomSchedPol() :
	dist(0, 100) {

	// Get a logger
	plugins::LoggerIF::Configuration conf(MODULE_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		if (daemonized)
			syslog(LOG_INFO, "Build RANDOM schedpol plugin [%p] FAILED "
					"(Error: missing logger module)", (void*)this);
		else
			fprintf(stdout, FMT_INFO("Build RANDOM schedpol plugin [%p] FAILED "
					"(Error: missing logger module)\n"), (void*)this);
	}

	assert(logger);
	logger->Debug("Built RANDOM SchedPol object @%p", (void*)this);

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

void RandomSchedPol::ScheduleApp(AppCPtr_t papp) {
	ResourceAccounter &ra(ResourceAccounter::GetInstance());
	ba::WorkingMode::ExitCode_t bindResult;
	ba::AwmPtrList_t::const_iterator it;
	ba::AwmPtrList_t::const_iterator end;
	ba::AwmPtrList_t const *awms;
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
	bindResult = (*it)->BindResource("cluster", RSRC_ID_ANY, selected_cluster);
	if (bindResult != ba::WorkingMode::WM_SUCCESS) {
		logger->Error("Resource biding for EXC [%s] FAILED", papp->StrId());
		return;
	}

	// Schedule the selected AWM on the selected Cluster
	papp->ScheduleRequest((*it), ra_view);

}


SchedulerPolicyIF::ExitCode_t
RandomSchedPol::Schedule(bbque::System & sv, RViewToken_t &rav) {
	ResourceAccounter &ra(ResourceAccounter::GetInstance());
	ResourceAccounter::ExitCode_t viewResult;
	AppsUidMapIt app_it;
	AppCPtr_t papp;

	if (!logger) {
		assert(logger);
		return SCHED_ERROR;
	}

	// Get a new view on the ResourceAccounter
	viewResult = ra.GetView(MODULE_NAMESPACE, ra_view);
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

	// Pass back to the SchedulerManager a reference to the scheduled view
	rav = ra_view;
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

