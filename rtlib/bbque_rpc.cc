/**
 *       @file  bbque_rpc.h
 *      @brief  The Barbeque RPC interface to support comunicaiton among
 *      applications and the RTRM.
 *
 * This RPC mechanism is channel agnostic and defines a set of procedure that
 * applications could call to send requests to the Barbeque RTRM.  The actual
 * implementation of the communication channel is provided by class derived by
 * this one. This class provides also a factory method which allows to obtain
 * an instance to the concrete communication channel that can be selected at
 * compile time.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/11/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "bbque/rtlib/bbque_rpc.h"

#include "bbque/rtlib/rpc_fifo_client.h"
#include "bbque/app/application.h"

#include <cstdio>


#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "RTLIB_RPC  [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "RTLIB_RPC  [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "RTLIB_RPC  [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "RTLIB_RPC  [ERR]", fmt)

namespace ba = bbque::app;

namespace bbque { namespace rtlib {

BbqueRPC * BbqueRPC::GetInstance() {
	static BbqueRPC * instance = NULL;

	if (instance)
		return instance;

#ifdef BBQUE_RPC_FIFO
	DB(fprintf(stderr, FMT_DBG("Using FIFO RPC channel\n")));
	instance = new BbqueRPC_FIFO_Client();
#else
#error RPC Channel NOT defined
#endif

	return instance;
}

BbqueRPC::BbqueRPC(void) :
	initialized(false) {

}

BbqueRPC::~BbqueRPC(void) {
	excMap_t::iterator it;
	pregExCtx_t prec;

	DB(fprintf(stderr, FMT_DBG("BbqueRPC dtor\n")));

	// Dump out execution statistics
	it = exc_map.begin();
	if (it != exc_map.end()) {

		DumpStatsHeader();
		for ( ; it != exc_map.end(); ++it) {
			prec = (*it).second;
			DumpStats(prec, true);
		}

	}

	// Clean-up all the registered EXCs
	exc_map.clear();
}

RTLIB_ExitCode_t BbqueRPC::Init(const char *name) {
	RTLIB_ExitCode_t exitCode;

	if (initialized) {
		fprintf(stderr, FMT_WRN("RTLIB already initialized for app [%d:%s]\n"),
				appTrdPid, name);
		return RTLIB_OK;
	}

	// Getting application PID
	appTrdPid = gettid();

	DB(fprintf(stderr, FMT_DBG("Initializing app [%d:%s]\n"),
				appTrdPid, name));

	exitCode = _Init(name);
	if (exitCode != RTLIB_OK) {
		fprintf(stderr, FMT_ERR("Initialization FAILED\n"));
		return exitCode;
	}

	initialized = true;

	DB(fprintf(stderr, FMT_DBG("Initialation DONE\n")));
	return RTLIB_OK;

}

uint8_t BbqueRPC::GetNextExcID() {
	static uint8_t exc_id = 0;
	excMap_t::iterator it = exc_map.find(exc_id);

	// Ensuring unicity of the Execution Context ID
	while (it != exc_map.end()) {
		exc_id++;
		it = exc_map.find(exc_id);
	}

	return exc_id;
}

RTLIB_ExecutionContextHandler_t BbqueRPC::Register(
		const char* name,
		const RTLIB_ExecutionContextParams_t* params) {
	RTLIB_ExitCode_t result;
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	pregExCtx_t prec;

	assert(initialized);
	assert(name && params);

	fprintf(stderr, FMT_INF("Registering EXC [%s]...\n"), name);

	if (!initialized) {
		fprintf(stderr, FMT_ERR("Registering EXC [%s] FAILED "
					"(Error: RTLIB not initialized)\n"), name);
		return NULL;
	}

	// Ensuring the execution context has not been already registered
	for( ; it != end; ++it) {
		prec = (*it).second;
		if (prec->name == name) {
			fprintf(stderr, FMT_ERR("Registering EXC [%s] FAILED "
				"(Error: EXC already registered)\n"), name);
			assert(prec->name != name);
			return NULL;
		}
	}

	// Build a new registered EXC
	prec = pregExCtx_t(new RegisteredExecutionContext_t(name, GetNextExcID()));
	memcpy((void*)&(prec->exc_params), (void*)params,
			sizeof(RTLIB_ExecutionContextParams_t));

	// Calling the Low-level registration
	result = _Register(prec);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Registering EXC [%s] FAILED "
			"(Error %d: %s)\n"),
			name, result, RTLIB_ErrorStr(result)));
		return NULL;
	}

	// Save the registered execution context
	exc_map.insert(excMapEntry_t(prec->exc_id, prec));

	// Mark the EXC as Registered
	setRegistered(prec);

	return (RTLIB_ExecutionContextHandler_t)&(prec->exc_params);
}

BbqueRPC::pregExCtx_t BbqueRPC::getRegistered(
		const RTLIB_ExecutionContextHandler_t ech) {
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	pregExCtx_t prec;

	assert(ech);

	// Checking for library initialization
	if (!initialized) {
		fprintf(stderr, FMT_ERR("EXC [%p] lookup FAILED "
			"(Error: RTLIB not initialized)\n"), (void*)ech);
		assert(initialized);
		return pregExCtx_t();
	}

	// Ensuring the execution context has been registered
	for( ; it != end; ++it) {
		prec = (*it).second;
		if ((void*)ech == (void*)&prec->exc_params)
			break;
	}
	if (it == end) {
		fprintf(stderr, FMT_ERR("EXC [%p] lookup FAILED "
			"(Error: EXC not registered)\n"), (void*)ech);
		assert(it != end);
		return pregExCtx_t();
	}

	return prec;
}

BbqueRPC::pregExCtx_t BbqueRPC::getRegistered(uint8_t exc_id) {
	excMap_t::iterator it(exc_map.find(exc_id));

	// Checking for library initialization
	if (!initialized) {
		fprintf(stderr, FMT_ERR("EXC [%d] lookup FAILED "
			"(Error: RTLIB not initialized)\n"), exc_id);
		assert(initialized);
		return pregExCtx_t();
	}

	if (it == exc_map.end()) {
		fprintf(stderr, FMT_ERR("EXC [%d] lookup FAILED "
			"(Error: EXC not registered)\n"), exc_id);
		assert(it != exc_map.end());
		return pregExCtx_t();
	}

	return (*it).second;
}

void BbqueRPC::Unregister(
		const RTLIB_ExecutionContextHandler_t ech) {
	RTLIB_ExitCode_t result;
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Unregister EXC [%p] FAILED "
				"(EXC not registered)\n"), (void*)ech);
		return;
	}

	assert(isRegistered(prec) == true);

	// Calling the low-level unregistration
	result = _Unregister(prec);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Unregister EXC [%p:%s] FAILED "
				"(Error %d: %s)\n"),
				(void*)ech, prec->name.c_str(), result,
				RTLIB_ErrorStr(result)));
		return;
	}

	// Dump (verbose) execution statistics
	DumpStats(prec);

	// Mark the EXC as Unregistered
	clearRegistered(prec);

}

void BbqueRPC::UnregisterAll() {
	RTLIB_ExitCode_t result;
	excMap_t::iterator it;
	pregExCtx_t prec;

	// Checking for library initialization
	if (!initialized) {
		fprintf(stderr, FMT_ERR("EXCs cleanup FAILED "
			"(Error: RTLIB not initialized)\n"));
		assert(initialized);
		return;
	}

	// Unregistering all the registered EXCs
	it = exc_map.begin();
	for ( ; it != exc_map.end(); ++it) {
		prec = (*it).second;

		// Jumping already un-registered EXC
		if (!isRegistered(prec))
				continue;

		// Calling the low-level unregistration
		result = _Unregister(prec);
		if (result != RTLIB_OK) {
			DB(fprintf(stderr, FMT_ERR("Unregister EXC [%s] FAILED "
							"(Error %d: %s)\n"),
						prec->name.c_str(), result,
						RTLIB_ErrorStr(result)));
			return;
		}

		// Mark the EXC as Unregistered
		clearRegistered(prec);
	}

}


RTLIB_ExitCode_t BbqueRPC::Enable(
		const RTLIB_ExecutionContextHandler_t ech) {
	RTLIB_ExitCode_t result;
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Enabling EXC [%p] FAILED "
				"(Error: EXC not registered)\n"), (void*)ech);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	assert(isEnabled(prec) == false);

	// Calling the low-level enable function
	result = _Enable(prec);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Enabling EXC [%p:%s] FAILED "
				"(Error %d: %s)\n"),
				(void*)ech, prec->name.c_str(), result,
				RTLIB_ErrorStr(result)));
		return RTLIB_EXC_ENABLE_FAILED;
	}

	// Mark the EXC as Enabled
	setEnabled(prec);

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::Disable(
		const RTLIB_ExecutionContextHandler_t ech) {
	RTLIB_ExitCode_t result;
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Disabling EXC [%p] STOP "
				"(Error: EXC not registered)\n"),
				(void*)ech);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	assert(isEnabled(prec) == true);

	// Calling the low-level disable function
	result = _Disable(prec);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Disabling EXC [%p:%s] FAILED "
				"(Error %d: %s)\n"),
				(void*)ech, prec->name.c_str(), result,
				RTLIB_ErrorStr(result)));
		return RTLIB_EXC_DISABLE_FAILED;
	}

	// Mark the EXC as Enabled
	clearEnabled(prec);

	// Dump statistics on EXC disabling
	DB(DumpStats(prec));

	// Unlocking eventually waiting GetWorkingMode
	prec->cv.notify_one();

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::SetupStatistics(pregExCtx_t prec) {
	assert(prec);
	pAwmStats_t pstats(prec->stats[prec->awm_id]);

	// Check if this is a newly selected AWM
	if (!pstats) {
		DB(fprintf(stderr, FMT_DBG("Setup stats for AWM [%d]\n"),
					prec->awm_id));
		pstats = prec->stats[prec->awm_id] =
			pAwmStats_t(new AwmStats_t);
		 
	}

	// Update usage count
	pstats->count++;

	// Configure current AWM stats
	prec->pAwmStats = pstats;
	
	return RTLIB_OK;
}

void BbqueRPC::DumpStats(pregExCtx_t prec, bool verbose) {
	AwmStatsMap_t::iterator it;
	pAwmStats_t pstats;
	uint8_t awm_id;

	if (verbose) {	
		fprintf(stderr, FMT_INF("+--- EXC:AWM ---|-- Uses --|"
				"-- Running [ms] --|-- Cycles --+\n"));
	} else {
		DB(fprintf(stderr, FMT_DBG("+--- EXC:AWM ---|-- Uses --|"
				"-- Running [ms] --|-- Cycles --+\n")));
	}
	it = prec->stats.begin();
	for ( ; it != prec->stats.end(); ++it) {
		awm_id = (*it).first;
		pstats = (*it).second;

	if (verbose) {	
		fprintf(stderr, FMT_INF("|%11s:%02hu | %8d | %16d | %10d |\n"),
			prec->name.c_str(), awm_id,
			pstats->count, pstats->time_processing, pstats->cycles);
	} else {
		DB(fprintf(stderr, FMT_DBG("|%11s:%02hu | %8d | %16d | %10d |\n"),
			prec->name.c_str(), awm_id,
			pstats->count, pstats->time_processing, pstats->cycles));
	}
	}

	if (verbose) {	
		fprintf(stderr, FMT_INF("+---------------+----------+"
				"------------------+------------+\n"));
	} else {
		DB(fprintf(stderr, FMT_DBG("+---------------+----------+"
				"------------------+------------+\n")));
	}
}

void BbqueRPC::SyncTimeEstimation(pregExCtx_t prec) {
	pAwmStats_t pstats(prec->pAwmStats);
	// Use high resolution to avoid errors on next computations
	double last_cycle_ms;
	
	// TIMER: Get RUNNING
	last_cycle_ms = prec->exc_tmr.getElapsedTimeMs();

	DB(fprintf(stderr, FMT_DBG("Last cycle time %10.3f[ms] for "
					"EXC [%s:%02hu]\n"),
				last_cycle_ms,
				prec->name.c_str(), prec->exc_id));

	// TIMER: Re-sart RUNNING
	prec->exc_tmr.start();

	// Update Cumulative Average:
	//                    t(n+1) - CA(n)
	// CA(n+1) = CA(n) + ----------------
	//                        (n+1)
	pstats->sync_time_ca = pstats->sync_time_ca + (
		(last_cycle_ms - pstats->sync_time_ca) /
		(pstats->cycles)
		);

	DB(fprintf(stderr, FMT_DBG("Estimated %d[ms] sync time for "
					"EXC [%s:%02hu]\n"),
				pstats->sync_time_ca,
				prec->name.c_str(), prec->exc_id));

	// Update running counters
	pstats->time_processing += last_cycle_ms;
	prec->time_processing += last_cycle_ms;

}

RTLIB_ExitCode_t BbqueRPC::UpdateStatistics(pregExCtx_t prec) {
	pAwmStats_t pstats(prec->pAwmStats);

	// Ensuring statistics have been properly setup
	assert(pstats);

	// Check if this is the first re-start on this AWM
	if (!isSyncDone(prec)) {
		// TIMER: Get RECONF
		prec->time_reconf += prec->exc_tmr.getElapsedTimeMs();
		// TIMER: Sart RUNNING
		prec->exc_tmr.start();
		return RTLIB_OK;
	}

	// Update CA for sync time estimation
	pstats->cycles++;

	// Update sync time estimation
	SyncTimeEstimation(prec);

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::GetAssignedWorkingMode(
		pregExCtx_t prec,
		RTLIB_WorkingModeParams_t *wm) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);

	if (!isEnabled(prec)) {
		DB(fprintf(stderr, FMT_DBG("Get AWM FAILED "
				"(Error: EXC not enabled)\n")));
		return RTLIB_EXC_GWM_FAILED;
	}

	if (isSyncMode(prec) && !isAwmValid(prec)) {
		DB(fprintf(stderr, FMT_DBG("SYNC Pending\n")));
		return RTLIB_EXC_SYNC_MODE;
	}

	if (!isAwmValid(prec)) {
		DB(fprintf(stderr, FMT_DBG("NOT valid AWM\n")));
		return RTLIB_EXC_GWM_FAILED;
	}

	DB(fprintf(stderr, FMT_DBG("Valid AWM assigned\n")));
	wm->awm_id = prec->awm_id;

	// Update AWM statistics
	UpdateStatistics(prec);

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::WaitForWorkingMode(
		pregExCtx_t prec,
		RTLIB_WorkingModeParams_t *wm) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);

	// Notify we are going to be suspended waiting for an AWM
	setAwmWaiting(prec);

	// TIMER: Start BLOCKED
	prec->exc_tmr.start();

	while (isEnabled(prec) && !isAwmValid(prec) && !isBlocked(prec))
		prec->cv.wait(rec_ul);

	clearAwmWaiting(prec);
	wm->awm_id = prec->awm_id;

	// Setup AWM statistics
	SetupStatistics(prec);

	// TIMER: Get BLOCKED
	prec->time_blocked += prec->exc_tmr.getElapsedTimeMs();

	// TIMER: Sart RECONF
	prec->exc_tmr.start();

	return RTLIB_OK;
}


RTLIB_ExitCode_t BbqueRPC::WaitForSyncDone(pregExCtx_t prec) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);

	while (isEnabled(prec) && !isSyncDone(prec))
		prec->cv.wait(rec_ul);

	// TODO add a timeout wait to limit the maximum reconfiguration time
	// before notifying an anomaly to the RTRM

	clearSyncMode(prec);
	return RTLIB_OK;
}


RTLIB_ExitCode_t BbqueRPC::GetWorkingMode(
		const RTLIB_ExecutionContextHandler_t ech,
		RTLIB_WorkingModeParams_t *wm,
		RTLIB_SyncType_t st) {
	RTLIB_ExitCode_t result;
	pregExCtx_t prec;
	// FIXME Remove compilation warning
	(void)st;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Getting WM for EXC [%p] FAILED "
			"(Error: EXC not registered)\n"),
			(void*)ech);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	if (unlikely(prec->ctrlTrdPid == 0)) {
		// Keep track of the Control Thread PID
		prec->ctrlTrdPid = gettid();
		DB(fprintf(stderr, FMT_DBG("Keep control thread PID [%d] "
						"for EXC [%d]...\n"),
					prec->ctrlTrdPid, prec->exc_id));
	}

	// Checking if a valid AWM has been assigned
	DB(fprintf(stderr, FMT_DBG("Looking for assigned AWM...\n")));
	result = GetAssignedWorkingMode(prec, wm);
	if (result == RTLIB_OK) {
		setSyncDone(prec);
		// Notify about synchronization completed
		prec->cv.notify_one();
		return RTLIB_OK;
	}

	// Exit if the EXC has been disabled
	if (!isEnabled(prec))
		return RTLIB_EXC_GWM_FAILED;

	if (result == RTLIB_EXC_GWM_FAILED) {
		DB(fprintf(stderr, FMT_DBG("AWM not assigned, "
					"sending schedule request to RTRM...\n")));
		DB(fprintf(stderr, FMT_INF(
				"[%s:%02hu] ===> BBQ::ScheduleRequest()\n"),
				prec->name.c_str(), prec->exc_id));
		// Calling the low-level start
		result = _ScheduleRequest(prec);
		if (result != RTLIB_OK)
			goto exit_gwm_failed;
	} else {
		// At this point, the EXC should be in Synchronization Mode
		// and thus it should wait for an EXC being assigned by the RTRM
		assert(result == RTLIB_EXC_SYNC_MODE);
	}

	DB(fprintf(stderr, FMT_DBG("Waiting for assigned AWM...\n")));

	// Waiting for an AWM being assigned
	result = WaitForWorkingMode(prec, wm);
	if (result != RTLIB_OK)
		goto exit_gwm_failed;

	// Exit if the EXC has been disabled
	if (!isEnabled(prec))
		return RTLIB_EXC_GWM_FAILED;

	// Processing the required reconfiguration action
	switch(prec->event) {
	case RTLIB_EXC_GWM_START:
	case RTLIB_EXC_GWM_RECONF:
	case RTLIB_EXC_GWM_MIGREC:
	case RTLIB_EXC_GWM_MIGRATE:
		DB(fprintf(stderr, FMT_INF(
				"[%s:%02hu] <------------- AWM [%02d] --\n"),
				prec->name.c_str(), prec->exc_id,
				prec->awm_id));
		break;
	case RTLIB_EXC_GWM_BLOCKED:
		DB(fprintf(stderr, FMT_INF(
				"[%s:%02hu] <---------------- BLOCKED --\n"),
				prec->name.c_str(), prec->exc_id));
		break;
	default:
		DB(fprintf(stderr, FMT_ERR("Execution context [%s] GWM FAILED "
					"(Error: Invalid event [%d])\n"),
				prec->name.c_str(), prec->event));
		assert(prec->event >= RTLIB_EXC_GWM_START);
		assert(prec->event <= RTLIB_EXC_GWM_BLOCKED);
		break;
	}

	return prec->event;

exit_gwm_failed:

	DB(fprintf(stderr, FMT_ERR("Execution context [%s] GWM FAILED "
					"(Error %d: %s)\n"),
				prec->name.c_str(), result,
				RTLIB_ErrorStr(result)));
	return RTLIB_EXC_GWM_FAILED;

}

uint32_t BbqueRPC::GetSyncLatency(pregExCtx_t prec) {
	(void)prec;
	// TODO added here the code for synchronization latency computation
	// By default now we return a 100[ms] synchronization latency value
	return 100;
}


/******************************************************************************
 * Synchronization Protocol Messages
 ******************************************************************************/

RTLIB_ExitCode_t BbqueRPC::SyncP_PreChangeNotify(pregExCtx_t prec) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);
	// Entering Synchronization mode
	setSyncMode(prec);
	// Resetting Sync Done
	clearSyncDone(prec);
	// Setting current AWM as invalid
	setAwmInvalid(prec);
	// Set application BLOCKED (if required)
	if (prec->event == RTLIB_EXC_GWM_BLOCKED)
		setBlocked(prec);
	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::SyncP_PreChangeNotify(
		rpc_msg_BBQ_SYNCP_PRECHANGE_t &msg) {
	RTLIB_ExitCode_t result;
	uint32_t syncLatency;
	pregExCtx_t prec;

	prec = getRegistered(msg.hdr.exc_id);
	if (!prec) {
		fprintf(stderr, FMT_ERR("SyncP_1 (Pre-Change) EXC [%d] FAILED "
				"(Error: Execution Context not registered)\n"),
				msg.hdr.exc_id);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	// Keep copy of the required synchronization action
	assert(msg.event < ba::ApplicationStatusIF::SYNC_STATE_COUNT);
	prec->event = (RTLIB_ExitCode_t)(RTLIB_EXC_GWM_START + msg.event);

	result = SyncP_PreChangeNotify(prec);

	// Set the new required AWM (if not being blocked)
	if (prec->event != RTLIB_EXC_GWM_BLOCKED) {
		prec->awm_id = msg.awm;
		fprintf(stderr, FMT_INF("SyncP_1 (Pre-Change) EXC [%d], "
					"Action [%d], Assigned AWM [%d]\n"), msg.hdr.exc_id,
				msg.event, msg.awm);
	} else {
		fprintf(stderr, FMT_INF("SyncP_1 (Pre-Change) EXC [%d], "
					"Action [%d:BLOCKED]\n"), msg.hdr.exc_id, msg.event);
	}

	// FIXME add a string representation of the required action

	// Update the Synchronziation Latency
	syncLatency = GetSyncLatency(prec);
	DB(fprintf(stderr, FMT_DBG("SyncP_1 (Pre-Change) EXC [%d], "
				"SyncLatency [%u]\n"),
				msg.hdr.exc_id, syncLatency));

	_SyncpPreChangeResp(msg.hdr.token, prec, syncLatency);

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::SyncP_SyncChangeNotify(pregExCtx_t prec) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);
	// Checking if the apps is in Sync Status
	if (!isAwmWaiting(prec))
		return RTLIB_EXC_SYNCP_FAILED;
	
	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::SyncP_SyncChangeNotify(
		rpc_msg_BBQ_SYNCP_SYNCCHANGE_t &msg) {
	RTLIB_ExitCode_t result;
	pregExCtx_t prec;

	prec = getRegistered(msg.hdr.exc_id);
	if (!prec) {
		fprintf(stderr, FMT_ERR("SyncP_2 (Sync-Change) EXC [%d] FAILED "
				"(Error: Execution Context not registered)\n"),
				msg.hdr.exc_id);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	result = SyncP_SyncChangeNotify(prec);
	if (result != RTLIB_OK) {
		fprintf(stderr, FMT_WRN("SyncP_2 (Sync-Change) EXC [%d] CRITICAL "
				"(Warning: Overpassing Synchronization time)\n"),
				msg.hdr.exc_id);
	}

	fprintf(stderr, FMT_INF("SyncP_2 (Sync-Change) EXC [%d]\n"),
			msg.hdr.exc_id);

	_SyncpSyncChangeResp(msg.hdr.token, prec, result);

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::SyncP_DoChangeNotify(pregExCtx_t prec) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);

	// Setting current AWM as valid
	setAwmValid(prec);

	// TODO Setup the ground for reconfiguration statistics collection
	// TODO Start the re-configuration by notifying the waiting thread
	prec->cv.notify_one();
	
	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::SyncP_DoChangeNotify(
		rpc_msg_BBQ_SYNCP_DOCHANGE_t &msg) {
	RTLIB_ExitCode_t result;
	pregExCtx_t prec;

	prec = getRegistered(msg.hdr.exc_id);
	if (!prec) {
		fprintf(stderr, FMT_ERR("SyncP_3 (Do-Change) EXC [%d] FAILED "
				"(Error: Execution Context not registered)\n"),
				msg.hdr.exc_id);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	result = SyncP_DoChangeNotify(prec);

	// NOTE this command should not generate a response, it is just a notification

	fprintf(stderr, FMT_INF("SyncP_3 (Do-Change) EXC [%d]\n"),
			msg.hdr.exc_id);

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::SyncP_PostChangeNotify(pregExCtx_t prec) {
	// TODO Wait for the apps to end its reconfiguration
	// TODO Collect stats on reconfiguraiton time

	DB(fprintf(stderr, FMT_DBG("Waiting for reconfiguration to complete...\n")));

	return WaitForSyncDone(prec);	
}

RTLIB_ExitCode_t BbqueRPC::SyncP_PostChangeNotify(
		rpc_msg_BBQ_SYNCP_POSTCHANGE_t &msg) {
	RTLIB_ExitCode_t result;
	pregExCtx_t prec;

	prec = getRegistered(msg.hdr.exc_id);
	if (!prec) {
		fprintf(stderr, FMT_ERR("SyncP_4 (Post-Change) EXC [%d] FAILED "
				"(Error: Execution Context not registered)\n"),
				msg.hdr.exc_id);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	result = SyncP_PostChangeNotify(prec);
	if (result != RTLIB_OK) {
		fprintf(stderr, FMT_WRN("SyncP_4 (Post-Change) EXC [%d] CRITICAL "
				"(Warning: Reconfiguration timeout)\n"),
				msg.hdr.exc_id);
	}

	fprintf(stderr, FMT_INF("SyncP_4 (Post-Change) EXC [%d]\n"),
			msg.hdr.exc_id);

	_SyncpPostChangeResp(msg.hdr.token, prec, result);

	return RTLIB_OK;
}

/******************************************************************************
 * Channel Independant interface
 ******************************************************************************/

RTLIB_ExitCode_t BbqueRPC::Set(
		const RTLIB_ExecutionContextHandler_t ech,
		RTLIB_Constraint_t* constraints,
		uint8_t count) {
	//Silence "args not used" warning.
	(void)ech;
	(void)constraints;
	(void)count;

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::Clear(
		const RTLIB_ExecutionContextHandler_t ech) {
	//Silence "args not used" warning.
	(void)ech;

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::StopExecution(
		RTLIB_ExecutionContextHandler_t ech,
		struct timespec timeout) {
	//Silence "args not used" warning.
	(void)ech;
	(void)timeout;

	return RTLIB_OK;
}

} // namespace rtlib

} // namespace bbque

