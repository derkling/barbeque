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
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
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
namespace bu = bbque::utils;

namespace bbque { namespace rtlib {

BbqueRPC * BbqueRPC::GetInstance() {
	static BbqueRPC * instance = NULL;

	if (instance)
		return instance;

	// Parse environment configuration
	ParseOptions();

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

RTLIB_ExitCode_t BbqueRPC::ParseOptions() {
	const char *env;
	char buff[32];
	char *opt;

	DB(fprintf(stderr, FMT_DBG("Parsing environment options...\n")));
	fprintf(stderr, FMT_DBG("Parsing environment options...\n"));

	// Look-for the expected RTLIB configuration variable
	env = getenv("BBQUE_RTLIB_OPTS");
	if (!env)
		return RTLIB_OK;

	DB(fprintf(stderr, FMT_DBG("BBQUE_RTLIB_OPTS: [%s]\n"), env));
	fprintf(stderr, FMT_DBG("BBQUE_RTLIB_OPTS: [%s]\n"), env);

	// Tokenize configuration options
	strncpy(buff, env, 32);
	opt = strtok(buff, ":");

	// Parsing all options (only single char opts are supported)
	while (opt) {

		DB(fprintf(stderr, FMT_DBG("OPT: %s\n"), opt));

		switch (opt[0]) {
		case 'b':
			// Enabling "big numbers" notations
			envBigNum = true;
			break;
		case 'c':
			// Enabling CSV output
			envCsvOutput = true;
			break;
		case 'G':
			// Enabling Global statistics collection
			envGlobal = true;
			break;
		case 'K':
			// Disable Kernel and Hipervisor from collected statistics
			envNoKernel = true;
			break;
		case 'O':
			// Collect statistics on RTLIB overheads
			envOverheads = true;
			break;
		case 'p':
			// Enabling perf...
			envPerfCount = true;
			// ... with the specified verbosity level
			sscanf(opt+1, "%d", &envDetailedRun);
			fprintf(stderr, "Enabling Perf Counters [verbosity: %d]\n",
					envDetailedRun);
			break;
		case 's':
			// Setting CSV separator
			envCsvSep = opt+1;
			break;
		}

		// Get next option
		opt = strtok(NULL, ":");
	}

	return RTLIB_OK;
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

		// Setup Performance Counters (if required)
		if (PerfRegisteredEvents(prec)) {
			PerfSetupStats(prec, pstats);
		}
	}

	// Update usage count
	pstats->count++;

	// Configure current AWM stats
	prec->pAwmStats = pstats;

	return RTLIB_OK;
}

#define STATS_HEADER \
"#                _----------------=> Uses#\n"\
"#               /    _------------=> Cycles#\n"\
"#              |    /        _----=> Processing [ms]\n"\
"#              |   |        /     |====== Sync Time [ms] ======|\n"\
"#   EXC  AWM   |   |       |       _(min,max)_      _(avg,var)_\n"\
"#     \\   /    |   |       |     /             \\  /             \\\n"\

void BbqueRPC::DumpStatsHeader() {
	fprintf(stderr, "\n");
	fprintf(stderr, STATS_HEADER);
}

void BbqueRPC::DumpStats(pregExCtx_t prec, bool verbose) {
	AwmStatsMap_t::iterator it;
	pAwmStats_t pstats;
	uint8_t awm_id;

	uint32_t _cycles;
	double _min;
	double _max;
	double _avg;
	double _var;

	it = prec->stats.begin();
	for ( ; it != prec->stats.end(); ++it) {
		awm_id = (*it).first;
		pstats = (*it).second;

		// Ignoring empty statistics
		_cycles = count(pstats->samples);
		if (!_cycles)
			continue;

		// Features extraction
		_min = min(pstats->samples);
		_max = max(pstats->samples);
		_avg = mean(pstats->samples);
		_var = variance(pstats->samples);

		if (verbose) {
			fprintf(stderr, "%8s-%d %5d %6d %7d "
					"(%7.3f,%7.3f)(%7.3f,%7.3f)\n",
				prec->name.c_str(), awm_id, pstats->count,
				_cycles, pstats->time_processing,
				_min, _max, _avg, _var);
		} else {
			DB(fprintf(stderr, FMT_DBG("%8s-%d %5d %6d %7d "
					"(%7.3f,%7.3f)(%7.3f,%7.3f)\n"),
				prec->name.c_str(), awm_id, pstats->count,
				_cycles, pstats->time_processing,
				_min, _max, _avg, _var));
		}

	}

	if (!PerfRegisteredEvents(prec) || !verbose)
		return;

	// Print performance counters for this AWM
	it = prec->stats.begin();
	for ( ; it != prec->stats.end(); ++it) {
		awm_id = (*it).first;
		pstats = (*it).second;

		_cycles = count(pstats->samples);
		fprintf(stderr, "\nPerf counters stats for '%s-%d' (%d cycles):\n\n",
				prec->name.c_str(), awm_id, _cycles);
		PerfPrintStats(prec, pstats);
	}

}

void BbqueRPC::SyncTimeEstimation(pregExCtx_t prec) {
	pAwmStats_t pstats(prec->pAwmStats);
	std::unique_lock<std::mutex> stats_ul(pstats->stats_mtx);
	// Use high resolution to avoid errors on next computations
	double last_cycle_ms;

	// TIMER: Get RUNNING
	last_cycle_ms = prec->exc_tmr.getElapsedTimeMs();

	DB(fprintf(stderr, FMT_DBG("Last cycle time %10.3f[ms] for "
					"EXC [%s:%02hu]\n"),
				last_cycle_ms,
				prec->name.c_str(), prec->exc_id));

	// Update running counters
	pstats->time_processing += last_cycle_ms;
	prec->time_processing += last_cycle_ms;

	// Push sample into accumulator
	pstats->samples(last_cycle_ms);

	// Statistic features extraction for cycle time estimation:
	DB(
	uint32_t _count = count(pstats->samples);
	double _min = min(pstats->samples);
	double _max = max(pstats->samples);
	double _avg = mean(pstats->samples);
	double _var = variance(pstats->samples);
	fprintf(stderr, FMT_DBG("#%08d: m: %.3f, M: %.3f, "
			"a: %.3f, v: %.3f) [ms]\n"),
		_count, _min, _max, _avg, _var);
	)

	// TODO: here we can get the overhead for statistica analysis
	// by re-reading the timer and comparing with the preivously readed
	// value

	// TIMER: Re-sart RUNNING
	prec->exc_tmr.start();

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

	if (isBlocked(prec)) {
		DB(fprintf(stderr, FMT_DBG("BLOCKED\n")));
		return RTLIB_EXC_GWM_BLOCKED;
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

	// Wait for the EXC being un-BLOCKED
	if (isBlocked(prec))
		while (isBlocked(prec))
			prec->cv.wait(rec_ul);
	else
	// Wait for the EXC being assigned an AWM
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

	while (isEnabled(prec) && !isSyncDone(prec)) {
		DB(fprintf(stderr, FMT_DBG("Waiting for reconfiguration to complete...\n")));
		prec->cv.wait(rec_ul);
	}

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

	// Checking if the EXC has been blocked
	if (result == RTLIB_EXC_GWM_BLOCKED) {
		setSyncDone(prec);
		// Notify about synchronization completed
		prec->cv.notify_one();
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
		// At this point, the EXC should be either in Synchronization Mode
		// or Blocked, and thus it should wait for an EXC being
		// assigned by the RTRM
		assert((result == RTLIB_EXC_SYNC_MODE) ||
				(result == RTLIB_EXC_GWM_BLOCKED));
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
	pAwmStats_t pstats = prec->pAwmStats;
	double elapsedTime;
	double syncDelay;
	double _avg;
	double _var;

	// Get the statistics for the current AWM
	assert(pstats);
	std::unique_lock<std::mutex> stats_ul(pstats->stats_mtx);
	_avg = mean(pstats->samples);
	_var = variance(pstats->samples);
	stats_ul.unlock();

	// Compute a reasonale sync point esimation
	// we assume a NORMAL distribution of execution times
	syncDelay = _avg + (10 * sqrt(_var));

	// Discount the already passed time since lasy sync point
	elapsedTime = prec->exc_tmr.getElapsedTimeMs();
	if (elapsedTime < syncDelay)
		syncDelay -= elapsedTime;
	else
		syncDelay = 0;

	DB(fprintf(stderr, FMT_DBG("Expected sync time in %10.3f[ms] for "
					"EXC [%s:%02hu]\n"),
				syncDelay,
				prec->name.c_str(), prec->exc_id));

	return ceil(syncDelay);
}


/******************************************************************************
 * Synchronization Protocol Messages
 ******************************************************************************/

RTLIB_ExitCode_t BbqueRPC::SyncP_PreChangeNotify(pregExCtx_t prec) {
	// Entering Synchronization mode
	setSyncMode(prec);
	// Resetting Sync Done
	clearSyncDone(prec);
	// Setting current AWM as invalid
	setAwmInvalid(prec);
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

	assert(msg.event < ba::ApplicationStatusIF::SYNC_STATE_COUNT);

	std::unique_lock<std::mutex> rec_ul(prec->mtx);

	// Keep copy of the required synchronization action
	prec->event = (RTLIB_ExitCode_t)(RTLIB_EXC_GWM_START + msg.event);

	result = SyncP_PreChangeNotify(prec);

	// Set the new required AWM (if not being blocked)
	if (prec->event != RTLIB_EXC_GWM_BLOCKED) {
		prec->awm_id = msg.awm;
		fprintf(stderr, FMT_INF("SyncP_1 (Pre-Change) EXC [%d], "
					"Action [%d], Assigned AWM [%d]\n"),
				msg.hdr.exc_id,
				msg.event, msg.awm);
	} else {
		fprintf(stderr, FMT_INF("SyncP_1 (Pre-Change) EXC [%d], "
					"Action [%d:BLOCKED]\n"),
				msg.hdr.exc_id, msg.event);
	}

	// FIXME add a string representation of the required action

	syncLatency = 0;
	if (!isAwmWaiting(prec) && prec->pAwmStats) {
		// Update the Synchronziation Latency
		syncLatency = GetSyncLatency(prec);
	}

	rec_ul.unlock();

	DB(fprintf(stderr, FMT_DBG("SyncP_1 (Pre-Change) EXC [%d], "
				"SyncLatency [%u]\n"),
				msg.hdr.exc_id, syncLatency));

	result = _SyncpPreChangeResp(msg.hdr.token, prec, syncLatency);

	return result;
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

	// Update the EXC status based on the last required re-configuration action
	if (prec->event == RTLIB_EXC_GWM_BLOCKED) {
		setBlocked(prec);
	} else {
		clearBlocked(prec);
		setAwmValid(prec);
	}

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

	return result;
}

RTLIB_ExitCode_t BbqueRPC::SyncP_PostChangeNotify(pregExCtx_t prec) {
	// TODO Wait for the apps to end its reconfiguration
	// TODO Collect stats on reconfiguraiton time
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
	RTLIB_ExitCode_t result;
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Constraining EXC [%p] "
				"(Error: EXC not registered)\n"), (void*)ech);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	// Calling the low-level enable function
	result = _Set(prec, constraints, count);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Constraining EXC [%p:%s] FAILED "
				"(Error %d: %s)\n"),
				(void*)ech, prec->name.c_str(), result,
				RTLIB_ErrorStr(result)));
		return RTLIB_EXC_ENABLE_FAILED;
	}

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC::Clear(
		const RTLIB_ExecutionContextHandler_t ech) {
	RTLIB_ExitCode_t result;
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Clear constraints for EXC [%p] "
				"(Error: EXC not registered)\n"), (void*)ech);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	// Calling the low-level enable function
	result = _Clear(prec);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Clear contraints for EXC [%p:%s] FAILED "
				"(Error %d: %s)\n"),
				(void*)ech, prec->name.c_str(), result,
				RTLIB_ErrorStr(result)));
		return RTLIB_EXC_ENABLE_FAILED;
	}

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

/*******************************************************************************
 *    Performance Monitoring Support
 ******************************************************************************/

BbqueRPC::PerfEventAttr_t BbqueRPC::default_events[] = {

  {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK },
  {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES },
  {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS },
  {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS },

  {PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES	},
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
  {PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND },
  {PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND },
#endif
  {PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS },
  {PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS },
  {PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES },

};


BbqueRPC::PerfEventAttr_t BbqueRPC::detailed_events[] = {

  {PERF_TYPE_HW_CACHE, L1DC_RA },
  {PERF_TYPE_HW_CACHE, L1DC_RM },
  {PERF_TYPE_HW_CACHE, LLC_RA },
  {PERF_TYPE_HW_CACHE, LLC_RM },

};


BbqueRPC::PerfEventAttr_t BbqueRPC::very_detailed_events[] = {

  {PERF_TYPE_HW_CACHE,	L1IC_RA },
  {PERF_TYPE_HW_CACHE,	L1IC_RM },
  {PERF_TYPE_HW_CACHE,	DTLB_RA },
  {PERF_TYPE_HW_CACHE,	DTLB_RM },
  {PERF_TYPE_HW_CACHE,	ITLB_RA },
  {PERF_TYPE_HW_CACHE,	ITLB_RM },

};

BbqueRPC::PerfEventAttr_t BbqueRPC::very_very_detailed_events[] = {

  {PERF_TYPE_HW_CACHE,	L1DC_PA },
  {PERF_TYPE_HW_CACHE,	L1DC_PM },

};

bool BbqueRPC::envPerfCount = false;
bool BbqueRPC::envGlobal = false;
bool BbqueRPC::envOverheads = false;
int  BbqueRPC::envDetailedRun = 0;
bool BbqueRPC::envNoKernel = false;
bool BbqueRPC::envCsvOutput = false;
bool BbqueRPC::envBigNum = false;
const char *BbqueRPC::envCsvSep = " ";

BbqueRPC::pPerfEventStats_t BbqueRPC::PerfGetEventStats(pAwmStats_t pstats,
		perf_type_id type, uint64_t config) {
	PerfEventStatsMapByConf_t::iterator it;
	pPerfEventStats_t ppes;
	uint8_t conf_key, curr_key;

	conf_key = (uint8_t)(0xFF & config);

	// LookUp for requested event
	it = pstats->events_conf_map.lower_bound(conf_key);
	for ( ; it != pstats->events_conf_map.end(); ++it) {
		ppes = (*it).second;

		// Check if current (conf) key is still valid
		curr_key = (uint8_t)(0xFF & ppes->pattr->config);
		if (curr_key != conf_key)
			break;

		// Check if we found the required event
		if ((ppes->pattr->type == type) &&
			(ppes->pattr->config == config))
			return ppes;
	}

	return pPerfEventStats_t();
}

void BbqueRPC::PerfSetupEvents(pregExCtx_t prec) {
	uint8_t tot_counters = ARRAY_SIZE(default_events);
	int fd;

	// TODO get required Perf configuration from environment
	// to add eventually more detailed counters or to completely disable perf
	// support

	// Adding default events
	for (uint8_t e = 0; e < tot_counters; e++) {
		fd = prec->perf.AddCounter(
				default_events[e].type, default_events[e].config,
				envNoKernel);
		prec->events_map[fd] = &(default_events[e]);
	}

    if (envDetailedRun <  1)
        return;

    // Append detailed run extra attributes
	for (uint8_t e = 0; e < ARRAY_SIZE(detailed_events); e++) {
		fd = prec->perf.AddCounter(
				detailed_events[e].type, detailed_events[e].config,
				envNoKernel);
		prec->events_map[fd] = &(detailed_events[e]);
    }

    if (envDetailedRun <  2)
        return;

    // Append detailed run extra attributes
	for (uint8_t e = 0; e < ARRAY_SIZE(very_detailed_events); e++) {
		fd = prec->perf.AddCounter(
				very_detailed_events[e].type, very_detailed_events[e].config,
				envNoKernel);
		prec->events_map[fd] = &(very_detailed_events[e]);
    }

    if (envDetailedRun <  3)
        return;

    // Append detailed run extra attributes
	for (uint8_t e = 0; e < ARRAY_SIZE(very_very_detailed_events); e++) {
		fd = prec->perf.AddCounter(
				very_very_detailed_events[e].type, very_very_detailed_events[e].config,
				envNoKernel);
		prec->events_map[fd] = &(very_very_detailed_events[e]);
    }
}

void BbqueRPC::PerfSetupStats(pregExCtx_t prec, pAwmStats_t pstats) {
	PerfRegisteredEventsMap_t::iterator it;
	pPerfEventStats_t ppes;
	pPerfEventAttr_t ppea;
	uint8_t conf_key;
	int fd;

	if (PerfRegisteredEvents(prec) == 0)
		return;

	// Setup performance counters
	for (it = prec->events_map.begin(); it != prec->events_map.end(); ++it) {
		fd = (*it).first;
		ppea = (*it).second;

		// Build new perf counter statistics
		ppes = pPerfEventStats_t(new PerfEventStats_t());
		assert(ppes);
		ppes->id = fd;
		ppes->pattr = ppea;

		// Keep track of perf statistics for this AWM
		pstats->events_map[fd] = ppes;

		// Index statistics by configuration (radix)
		conf_key = (uint8_t)(0xFF & ppea->config);
		pstats->events_conf_map.insert(
			PerfEventStatsMapByConfEntry_t(conf_key, ppes));
	}

}

void BbqueRPC::PerfCollectStats(pregExCtx_t prec) {
	std::unique_lock<std::mutex> stats_ul(prec->pAwmStats->stats_mtx);
	pAwmStats_t pstats = prec->pAwmStats;
	PerfEventStatsMap_t::iterator it;
	pPerfEventStats_t ppes;
	uint64_t delta;
	int fd;

	// Collect counters for registered events
	it = pstats->events_map.begin();
	for ( ; it != pstats->events_map.end(); ++it) {
		ppes = (*it).second;
		fd = (*it).first;

		// Reading delta for this perf counter
		delta = prec->perf.Update(fd);

		// Computing stats for this counter
		ppes->value += delta;
		ppes->samples(delta);
	}

}

void BbqueRPC::PerfPrintNsec(pAwmStats_t pstats, pPerfEventStats_t ppes) {
	pPerfEventAttr_t ppea = ppes->pattr;
	double avg = mean(ppes->samples);
	double total, ratio = 0.0;
    double msecs = avg / 1e6;

    fprintf(stderr, "%19.6f%s%-25s", msecs, envCsvSep,
			bu::Perf::EventName(ppea->type, ppea->config));

	if (envCsvOutput)
        return;

	if (PerfEventMatch(ppea, PERF_SW(TASK_CLOCK))) {
		// Get AWM average running time
		total = mean(pstats->samples) * 1e6;

		if (total) {
			ratio = avg / total;
			fprintf(stderr, " # %8.3f CPUs utilized          ", ratio);
			return;
		}
	}

}

void BbqueRPC::PerfPrintMissesRatio(double avg_missed, double tot_branches, const char *text) {
	double ratio = 0.0;
	const char *color;

	if (tot_branches)
		ratio = avg_missed / tot_branches * 100.0;

	color = PERF_COLOR_NORMAL;
	if (ratio > 20.0)
		color = PERF_COLOR_RED;
	else if (ratio > 10.0)
		color = PERF_COLOR_MAGENTA;
	else if (ratio > 5.0)
		color = PERF_COLOR_YELLOW;

	fprintf(stderr, " #  ");
	bu::Perf::FPrintf(stderr, color, "%6.2f%%", ratio);
	fprintf(stderr, " %-23s", text);

}

void BbqueRPC::PerfPrintAbs(pAwmStats_t pstats, pPerfEventStats_t ppes) {
	pPerfEventAttr_t ppea = ppes->pattr;
	double avg = mean(ppes->samples);
	double total, total2, ratio = 0.0;
	pPerfEventStats_t ppes2;
	const char *fmt;

	// shutdown compiler warnings for kernels < 3.1
	(void)total2;

	if (envCsvOutput)
		fmt = "%.0f%s%s";
	else if (envBigNum)
		fmt = "%'19.0f%s%-25s";
	else
		fmt = "%19.0f%s%-25s";

	fprintf(stderr, fmt, avg, envCsvSep,
			bu::Perf::EventName(ppea->type, ppea->config));

    if (envCsvOutput)
        return;

	if (PerfEventMatch(ppea, PERF_HW(INSTRUCTIONS))) {

		// Compute "instructions per cycle"
		ppes2 = PerfGetEventStats(pstats, PERF_HW(CPU_CYCLES));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);

		if (total)
			ratio = avg / total;

		fprintf(stderr, " #   %5.2f  insns per cycle        ", ratio);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
		// Compute "stalled cycles per instruction"
		ppes2 = PerfGetEventStats(pstats, PERF_HW(STALLED_CYCLES_FRONTEND));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);

		ppes2 = PerfGetEventStats(pstats, PERF_HW(STALLED_CYCLES_BACKEND));
		if (!ppes2)
			return;
		total2 = mean(ppes2->samples);
		if (total < total2)
			total = total2;

		if (total && avg) {
			ratio = total / avg;
			fprintf(stderr, "\n%45s#   %5.2f  stalled cycles per insn", " ", ratio);
		}
#endif
		return;
	}

	if (PerfEventMatch(ppea, PERF_HW(BRANCH_MISSES))) {
		ppes2 = PerfGetEventStats(pstats, PERF_HW(BRANCH_INSTRUCTIONS));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);
		if (total)
			PerfPrintMissesRatio(avg, total, "of all branches");

		return;
	}

	if (PerfEventMatch(ppea, PERF_HC(L1DC_RM))) {
		ppes2 = PerfGetEventStats(pstats, PERF_HC(L1DC_RA));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);
		if (total)
			PerfPrintMissesRatio(avg, total, "of all L1-dcache hits");

		return;
	}

	if (PerfEventMatch(ppea, PERF_HC(L1IC_RM))) {
		ppes2 = PerfGetEventStats(pstats, PERF_HC(L1IC_RA));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);
		if (total)
			PerfPrintMissesRatio(avg, total, "of all L1-icache hits");

		return;
	}

	if (PerfEventMatch(ppea, PERF_HC(DTLB_RM))) {
		ppes2 = PerfGetEventStats(pstats, PERF_HC(DTLB_RA));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);
		if (total)
			PerfPrintMissesRatio(avg, total, "of all dTLB cache hits");

		return;
	}

	if (PerfEventMatch(ppea, PERF_HC(ITLB_RM))) {
		ppes2 = PerfGetEventStats(pstats, PERF_HC(ITLB_RA));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);
		if (total)
			PerfPrintMissesRatio(avg, total, "of all iTLB cache hits");

		return;
	}

	if (PerfEventMatch(ppea, PERF_HC(LLC_RM))) {
		ppes2 = PerfGetEventStats(pstats, PERF_HC(LLC_RA));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);
		if (total)
			PerfPrintMissesRatio(avg, total, "of all LL-cache hits");

		return;
	}

	if (PerfEventMatch(ppea, PERF_HW(CACHE_MISSES))) {
		ppes2 = PerfGetEventStats(pstats, PERF_HW(CACHE_REFERENCES));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);
		if (total) {
			ratio = avg * 100 / total;
		    fprintf(stderr, " # %8.3f %% of all cache refs    ", ratio);
		}

		return;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
	// TODO add CPU front/back-end stall stats
#endif

	if (PerfEventMatch(ppea, PERF_HW(CPU_CYCLES))) {
		ppes2 = PerfGetEventStats(pstats, PERF_SW(TASK_CLOCK));
		if (!ppes2)
			return;
		total = mean(ppes2->samples);
		if (total) {
			ratio = 1.0 * avg / total;
			fprintf(stderr, " # %8.3f GHz                    ", ratio);
		}

		return;
	}

	// By default print the frequency of the event in [M/sec]
	ppes2 = PerfGetEventStats(pstats, PERF_SW(TASK_CLOCK));
	if (!ppes2)
		return;
	total = mean(ppes2->samples);
	if (total) {
		ratio = 1000.0 * avg / total;
		fprintf(stderr, " # %8.3f M/sec                  ", ratio);
		return;
	}

	// Otherwise, simply generate an empty line
	fprintf(stderr, "%-35s", " ");

}

bool BbqueRPC::IsNsecCounter(pregExCtx_t prec, int fd) {
	pPerfEventAttr_t ppea = prec->events_map[fd];
	if (PerfEventMatch(ppea, PERF_SW(CPU_CLOCK)) ||
		PerfEventMatch(ppea, PERF_SW(TASK_CLOCK)))
		return true;
	return false;
}

void BbqueRPC::PerfPrintStats(pregExCtx_t prec, pAwmStats_t pstats) {
	PerfEventStatsMap_t::iterator it;
	pPerfEventStats_t ppes;
	uint64_t avg_enabled, avg_running;
	double avg_value, std_value;
	int fd;

	// For each registered counter
	for (it = pstats->events_map.begin(); it != pstats->events_map.end(); ++it) {
		ppes = (*it).second;
		fd = (*it).first;

		if (IsNsecCounter(prec, fd))
			PerfPrintNsec(pstats, ppes);
		else
			PerfPrintAbs(pstats, ppes);

		// Print stddev ratio
		if (count(ppes->samples) > 1) {

			// Get AWM average and stddev running time
			avg_value = mean(ppes->samples);
			std_value = sqrt(variance(ppes->samples));

			PrintNoisePct(std_value, avg_value);
		}

		// Computing counter scheduling statistics
		avg_enabled = prec->perf.Enabled(ppes->id, false);
		avg_running = prec->perf.Running(ppes->id, false);

		DB(fprintf(stderr, " (Ena: %20lu, Run: %10lu) ", avg_enabled, avg_running));

		// Print percentage of counter usage
		if (avg_enabled != avg_running)
			fprintf(stderr, " [%5.2f%%]", 100.0 * avg_running / avg_enabled);

		fputc('\n', stderr);
	}

	if (!envCsvOutput) {

		fputc('\n', stderr);

		// Get AWM average and stddev running time
		avg_value = mean(pstats->samples);
		std_value = sqrt(variance(pstats->samples));

		fprintf(stderr, " %18.6f seconds time elapsed", avg_value);
		if (count(pstats->samples) > 1) {
			fprintf(stderr, "                                        ");
			PrintNoisePct(std_value, avg_value);
		}
		fprintf(stderr, "\n\n");
	}

}

void BbqueRPC::PrintNoisePct(double total, double avg) {
	const char *color;
	double pct = 0.0;

	if (avg)
		pct = 100.0*total/avg;

	if (envCsvOutput) {
		fprintf(stderr, "%s%.2f%%", envCsvSep, pct);
		return;
	}

	color = PERF_COLOR_NORMAL;
	if (pct > 80.0)
		color = PERF_COLOR_RED;
	else if (pct > 60.0)
		color = PERF_COLOR_MAGENTA;
	else if (pct > 40.0)
		color = PERF_COLOR_YELLOW;

	fprintf(stderr, "  ( ");
	bu::Perf::FPrintf(stderr, color, "+-%6.2f%%", pct);
	fprintf(stderr, " )");
}

void BbqueRPC::NotifySetup(
	RTLIB_ExecutionContextHandler_t ech) {
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Unregister EXC [%p] FAILED "
				"(EXC not registered)\n"), (void*)ech);
		return;
	}

	assert(isRegistered(prec) == true);

	// Add all the required performance counters
	if (envPerfCount)
		PerfSetupEvents(prec);

}

void BbqueRPC::NotifyInit(
	RTLIB_ExecutionContextHandler_t ech) {
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Unregister EXC [%p] FAILED "
				"(EXC not registered)\n"), (void*)ech);
		return;
	}

	assert(isRegistered(prec) == true);

	if (envGlobal && PerfRegisteredEvents(prec)) {
		PerfEnable(prec);
	}

}

void BbqueRPC::NotifyExit(
	RTLIB_ExecutionContextHandler_t ech) {
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Unregister EXC [%p] FAILED "
				"(EXC not registered)\n"), (void*)ech);
		return;
	}

	assert(isRegistered(prec) == true);

	if (envGlobal && PerfRegisteredEvents(prec)) {
		PerfDisable(prec);
		PerfCollectStats(prec);
	}

}

void BbqueRPC::NotifyPreConfigure(
	RTLIB_ExecutionContextHandler_t ech) {
	DB(fprintf(stderr, FMT_DBG("===> NotifyConfigure\n")));
	(void)ech;
}

void BbqueRPC::NotifyPostConfigure(
	RTLIB_ExecutionContextHandler_t ech) {
	DB(fprintf(stderr, FMT_DBG("<=== NotifyConfigure\n")));
	(void)ech;
}

void BbqueRPC::NotifyPreRun(
	RTLIB_ExecutionContextHandler_t ech) {
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Unregister EXC [%p] FAILED "
				"(EXC not registered)\n"), (void*)ech);
		return;
	}

	assert(isRegistered(prec) == true);

	DB(fprintf(stderr, FMT_DBG("===> NotifyRun\n")));
	if (!envGlobal && PerfRegisteredEvents(prec)) {
		if (unlikely(envOverheads)) {
			PerfDisable(prec);
			PerfCollectStats(prec);
		} else
			PerfEnable(prec);
	}

}

void BbqueRPC::NotifyPostRun(
	RTLIB_ExecutionContextHandler_t ech) {
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Unregister EXC [%p] FAILED "
				"(EXC not registered)\n"), (void*)ech);
		return;
	}

	assert(isRegistered(prec) == true);

	DB(fprintf(stderr, FMT_DBG("<=== NotifyRun\n")));

	if (!envGlobal && PerfRegisteredEvents(prec)) {
		if (unlikely(envOverheads))
			PerfEnable(prec);
		else {
			PerfDisable(prec);
			PerfCollectStats(prec);
		}
	}

}

void BbqueRPC::NotifyPreMonitor(
	RTLIB_ExecutionContextHandler_t ech) {
	DB(fprintf(stderr, FMT_DBG("===> NotifyMonitor\n")));
	(void)ech;
}

void BbqueRPC::NotifyPostMonitor(
	RTLIB_ExecutionContextHandler_t ech) {
	DB(fprintf(stderr, FMT_DBG("<=== NotifyMonitor\n")));
	(void)ech;
}

void BbqueRPC::NotifyPreSuspend(
	RTLIB_ExecutionContextHandler_t ech) {
	DB(fprintf(stderr, FMT_DBG("===> NotifySuspend\n")));
	(void)ech;
}

void BbqueRPC::NotifyPostSuspend(
	RTLIB_ExecutionContextHandler_t ech) {
	DB(fprintf(stderr, FMT_DBG("<=== NotifySuspend\n")));
	(void)ech;
}

void BbqueRPC::NotifyPreResume(
	RTLIB_ExecutionContextHandler_t ech) {
	DB(fprintf(stderr, FMT_DBG("===> NotifyResume\n")));
	(void)ech;
}

void BbqueRPC::NotifyPostResume(
	RTLIB_ExecutionContextHandler_t ech) {
	DB(fprintf(stderr, FMT_DBG("<=== NotifyResume\n")));
	(void)ech;
}

} // namespace rtlib

} // namespace bbque

