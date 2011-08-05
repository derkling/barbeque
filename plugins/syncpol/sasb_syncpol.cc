/**
 *       @file  sasb_syncpol.cc
 *      @brief  The SASB synchronization policy
 *
 * This defines a dynamic C++ plugin which implements the "Starvation Avoidance
 * State Based" (SASB) heuristic for EXCc synchronizaiton.
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

#include "sasb_syncpol.h"

#include "bbque/modules_factory.h"

#include <iostream>
#include <random>

namespace bbque { namespace plugins {

SasbSyncPol::SasbSyncPol() :
	status(STEP10) {

	// Get a logger
	plugins::LoggerIF::Configuration conf(
			SYNCHRONIZATION_POLICY_NAMESPACE
			SYNCHRONIZATION_POLICY_NAME);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		std::cout << "SASB: Build sasb synchronization policy "
			<< this << "] FAILED (Error: missing logger module)" << std::endl;
		assert(logger);
	}

	logger->Debug("Built a new dynamic object[%p]\n", this);

}

SasbSyncPol::~SasbSyncPol() {
}

//----- Scheduler policy module interface

char const *SasbSyncPol::Name() {
	return SYNCHRONIZATION_POLICY_NAME;
}


ApplicationStatusIF::SyncState_t SasbSyncPol::step1(
			bbque::SystemView & sv) {

	logger->Debug("STEP 1.0: Running => Blocked");
	if (sv.HasApplications(ApplicationStatusIF::BLOCKED))
		return ApplicationStatusIF::BLOCKED;

	logger->Debug("STEP 1.0:            "
			"No EXCs to be BLOCKED");
	return ApplicationStatusIF::SYNC_NONE;
}

ApplicationStatusIF::SyncState_t SasbSyncPol::step2(
			bbque::SystemView & sv) {
	ApplicationStatusIF::SyncState_t syncState;

	switch(status) {
	case STEP21:
		logger->Debug("STEP 2.1: Running => Migration (lower prio)");
		syncState = ApplicationStatusIF::MIGRATE;
		break;

	case STEP22:
		logger->Debug("STEP 2.2: Running => Migration/Reconf (lower prio)");
		syncState = ApplicationStatusIF::MIGREC;
		break;

	case STEP23:
		logger->Debug("STEP 2.3: Running => Reconf (lower prio)");
		syncState = ApplicationStatusIF::RECONF;
		break;

	default:
		// We should never got here
		assert(false);
	}

	if (sv.HasApplications(syncState))
		return syncState;

	logger->Debug("STEP 2.0:            "
			"No EXCs to be reschedule (lower prio)");
	return ApplicationStatusIF::SYNC_NONE;
}


ApplicationStatusIF::SyncState_t SasbSyncPol::step3(
			bbque::SystemView & sv) {
	ApplicationStatusIF::SyncState_t syncState;

	switch(status) {
	case STEP31:
		logger->Debug("STEP 3.1: Running => Migration (higher prio)");
		syncState = ApplicationStatusIF::MIGRATE;
		break;

	case STEP32:
		logger->Debug("STEP 3.2: Running => Migration/Reconf (higher prio)");
		syncState = ApplicationStatusIF::MIGREC;
		break;

	case STEP33:
		logger->Debug("STEP 3.3: Running => Reconf (higher prio)");
		syncState = ApplicationStatusIF::RECONF;
		break;

	default:
		// We should never got here
		assert(false);
	}

	if (sv.HasApplications(syncState))
		return syncState;

	logger->Debug("STEP 3.0:            "
			"No EXCs to be reschedule (higher prio)");
	return ApplicationStatusIF::SYNC_NONE;
}

ApplicationStatusIF::SyncState_t SasbSyncPol::step4(
			bbque::SystemView & sv) {

	logger->Debug("STEP 4.0: Ready   => Running");
	if (sv.HasApplications(ApplicationStatusIF::STARTING))
		return ApplicationStatusIF::STARTING;

	logger->Debug("STEP 4.0:            "
			"No EXCs to be started");
	return ApplicationStatusIF::SYNC_NONE;
}

ApplicationStatusIF::SyncState_t SasbSyncPol::GetApplicationsQueue(
			bbque::SystemView & sv, bool restart) {
	ApplicationStatusIF::SyncState_t syncState;

	if (restart) {
		logger->Debug("Resetting sync status");
		status = STEP10;
	}

	// Resetting the maximum latency since a new queue is going to be served,
	// thus a new SyncP is going to start
	max_latency = 0;

	for( ; status<=STEP40; ++status) {
		switch(status) {
		case STEP10:
			syncState = step1(sv);
			if (syncState != ApplicationStatusIF::SYNC_NONE)
				return syncState;
			continue;
		case STEP21:
		case STEP22:
		case STEP23:
			syncState = step2(sv);
			if (syncState != ApplicationStatusIF::SYNC_NONE)
				return syncState;
			continue;
		case STEP31:
		case STEP32:
		case STEP33:
			syncState = step3(sv);
			if (syncState != ApplicationStatusIF::SYNC_NONE)
				return syncState;
			break;
		case STEP40:
			syncState = step4(sv);
			if (syncState != ApplicationStatusIF::SYNC_NONE)
				return syncState;
			break;
		};
	}

	return ApplicationStatusIF::SYNC_NONE;

}

bool SasbSyncPol::DoSync(AppPtr_t papp) {
	(void)papp;
	return true;
}

SasbSyncPol::ExitCode_t
SasbSyncPol::CheckLatency(AppPtr_t papp, SyncLatency_t latency) {
	// TODO: use a smarter latency validation, e.g. considering the
	// application and the currently served queue
	logger->Warn("TODO: Check for (%d[ms]) syncLatency compliance",
			latency);

	// Right now we use a dummy approach based on WORST CASE.
	// Indeed, we keep the maximum required lantecy among all the applications
	// since the last GetApplicationsQueue
	if (max_latency < latency)
		max_latency = latency;

	return SYNCP_OK;
}

SasbSyncPol::SyncLatency_t
SasbSyncPol::EstimatedSyncTime() {

	// TODO use a smarter latency estimation, e.g. based on a timer which
	// should be started at each first CheckLatency right after each new
	// GetApplicationsQueue

	// Right now we use a dummy approach based on WORT CASE.
	// Indeet we alwasy return the maximum latency collected among all the
	// applciations.
	return max_latency;
}


//----- static plugin interface

void * SasbSyncPol::Create(PF_ObjectParams *) {
	return new SasbSyncPol();
}

int32_t SasbSyncPol::Destroy(void * plugin) {
  if (!plugin)
    return -1;
  delete (SasbSyncPol *)plugin;
  return 0;
}

} // namesapce plugins

} // namespace bbque

