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
#include "bbque/utils/utility.h"

#include <cstdio>


#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "RTLIB_RPC  [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "RTLIB_RPC  [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "RTLIB_RPC  [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "RTLIB_RPC  [ERR]", fmt)

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
	DB(fprintf(stderr, FMT_DBG("BbqueRPC dtor\n")));
}

RTLIB_ExitCode BbqueRPC::Init(const char *name) {
	RTLIB_ExitCode exitCode;

	if (initialized) {
		fprintf(stderr, FMT_WRN("RTLIB already initialized for app [%s]\n"),
				name);
		return RTLIB_OK;
	}

	DB(fprintf(stderr, FMT_DBG("Initializing app [%s]\n"), name));

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

RTLIB_ExecutionContextHandler BbqueRPC::Register(
		const char* name,
		const RTLIB_ExecutionContextParams* params) {
	RTLIB_ExitCode result;
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	pregExCtx_t prec;

	assert(initialized);
	assert(name && params);
	assert(params->StopExecution);

	if (!initialized) {
		fprintf(stderr, FMT_ERR("Execution context [%s] registration FAILED "
					"(RTLIB not initialized)\n"), name);
		return NULL;
	}

	// Ensuring the execution context has not been already registered
	for( ; it != end; ++it) {
		prec = (*it).second;
		if (prec->name == name) {
			fprintf(stderr, FMT_ERR("Execution context [%s] already "
						"registered\n"), name);
			assert(prec->name != name);
			return NULL;
		}
	}

	// Build a new registered EXC
	prec = pregExCtx_t(new RegisteredExecutionContext_t);
	memcpy((void*)&(prec->exc_params), (void*)params,
			sizeof(RTLIB_ExecutionContextParams));
	prec->name = name;
	prec->exc_id = GetNextExcID();

	// Calling the Low-level registration
	result = _Register(prec);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Execution context [%s] REGISTRATION FAILED "
						"(Error %d)\n"), name, result));
		return NULL;
	}

	// Save the registered execution context
	exc_map.insert(excMapEntry_t(prec->exc_id, prec));

	return (RTLIB_ExecutionContextHandler)&(prec->exc_params);
}

BbqueRPC::pregExCtx_t BbqueRPC::getRegistered(
		const RTLIB_ExecutionContextHandler ech) {
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	pregExCtx_t prec;

	assert(ech);

	// Checking for library initialization
	if (!initialized) {
		fprintf(stderr, FMT_ERR("Execution context lookup FAILED "
					"(RTLIB not initialized)\n"));
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
		fprintf(stderr, FMT_ERR("Execution context [%p] NOT registered\n"),
				(void*)ech);
		assert(it != end);
		return pregExCtx_t();
	}

	return prec;
}

BbqueRPC::pregExCtx_t BbqueRPC::getRegistered(uint8_t exc_id) {
	excMap_t::iterator it(exc_map.find(exc_id));

	// Checking for library initialization
	if (!initialized) {
		fprintf(stderr, FMT_ERR("Execution context lookup FAILED "
					"(RTLIB not initialized)\n"));
		assert(initialized);
		return pregExCtx_t();
	}

	if (it == exc_map.end()) {
		fprintf(stderr, FMT_ERR("Execution context [%d] "
					"NOT registered\n"), exc_id);
		assert(it != exc_map.end());
		return pregExCtx_t();
	}

	return (*it).second;
}

void BbqueRPC::Unregister(
		const RTLIB_ExecutionContextHandler ech) {
	RTLIB_ExitCode result;
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Execution context [%p] UNREGISTRATION FAILED "
					"(Execution Context not registered)\n"), (void*)ech);
		return;
	}

	// Calling the low-level unregistration
	result = _Unregister(prec);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Execution context [%s] "
						"UNREGISTRATION FAILED (Error %d)\n"),
					prec->name.c_str(), result));
		return;
	}

	// Unegistered the execution context
	exc_map.erase(prec->exc_id);

}

RTLIB_ExitCode BbqueRPC::Start(
		const RTLIB_ExecutionContextHandler ech) {
	RTLIB_ExitCode result;
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Execution context [%p] START FAILED "
					"(Execution Context not registered)\n"), (void*)ech);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	// Calling the low-level start
	result = _Start(prec);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Execution context [%s] START FAILED "
				"(Error: %d)\n"), prec->name.c_str(), result));
		return RTLIB_EXC_START_FAILED;
	}

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC::Stop(
		const RTLIB_ExecutionContextHandler ech) {
	RTLIB_ExitCode result;
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Execution context [%p] STOP FAILED "
				"(Error: Execution Context not registered)\n"),
				(void*)ech);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	// Calling the low-level start
	result = _Stop(prec);
	if (result != RTLIB_OK) {
		DB(fprintf(stderr, FMT_ERR("Execution context [%s] STOP FAILED "
				"(Error %d)\n"), prec->name.c_str(), result));
		return RTLIB_EXC_STOP_FAILED;
	}

	return RTLIB_OK;
}

bool BbqueRPC::Sync(
		const RTLIB_ExecutionContextHandler ech,
		const char *name,
		RTLIB_SyncType type) {
	//Silence "args not used" warning.
	(void)ech;
	(void)name;
	(void)type;

	// Go on with the current working mode
	return true;
}

RTLIB_ExitCode BbqueRPC::GetAssignedWorkingMode(
		pregExCtx_t prec,
		RTLIB_WorkingModeParams *wm) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);

	if (!isAwmValid(prec))
		return RTLIB_EXC_GWM_FAILED;

	wm->awm_id = prec->awm_id;

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC::WaitForWorkingMode(
		pregExCtx_t prec,
		RTLIB_WorkingModeParams *wm) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);

	// Notify we are going to be suspended waiting for an AWM
	setAwmWaiting(prec);

	while (!isAwmValid(prec))
		prec->cv.wait(rec_ul);

	clearAwmWaiting(prec);
	wm->awm_id = prec->awm_id;

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC::SetWorkingMode(
		pregExCtx_t prec,
		RTLIB_WorkingModeParams *wm) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);
	(void)wm;

	// TODO
	assert(false);

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC::GetWorkingMode(
		const RTLIB_ExecutionContextHandler ech,
		RTLIB_WorkingModeParams *wm) {
	RTLIB_ExitCode result;
	pregExCtx_t prec;

	assert(ech);

	prec = getRegistered(ech);
	if (!prec) {
		fprintf(stderr, FMT_ERR("Execution context [%p] GWM FAILED "
			"(Error: Execution Context not registered)\n"),
			(void*)ech);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	// Checking if a valid AWM has been assigned
	DB(fprintf(stderr, FMT_DBG("Looking for assigned AWM...\n")));
	result = GetAssignedWorkingMode(prec, wm);
	if (result == RTLIB_OK)
		return RTLIB_OK;

	DB(fprintf(stderr, FMT_DBG("AWM not assigned: "
					"Requesting Scheduling...\n")));

	// Calling the low-level start
	result = _ScheduleRequest(prec);
	if (result != RTLIB_OK)
		goto exit_gwm_failed;

	// Waiting for an AWM being assigned
	result = WaitForWorkingMode(prec, wm);
	if (result != RTLIB_OK)
		goto exit_gwm_failed;

	return RTLIB_OK;

exit_gwm_failed:

	DB(fprintf(stderr, FMT_ERR("Execution context [%s] GWM FAILED "
					"(Error: %d)\n"),
				prec->name.c_str(), result));
	return RTLIB_EXC_GWM_FAILED;

}

RTLIB_ExitCode BbqueRPC::SyncP_PreChangeNotify(pregExCtx_t prec) {
	std::unique_lock<std::mutex> rec_ul(prec->mtx);
	// Setting current AWM as invalid
	setAwmInvalid(prec);
	return RTLIB_OK;
}

uint32_t BbqueRPC::GetSyncLatency(pregExCtx_t prec) {
	(void)prec;
	// TODO added here the code for synchronization latency computation
	// By default now we return a 100[ms] synchronization latency value
	return 100;
}

RTLIB_ExitCode BbqueRPC::SyncP_PreChangeNotify(
		rpc_msg_token_t token,
		uint8_t exc_id) {
	RTLIB_ExitCode result;
	uint32_t syncLatency;
	pregExCtx_t prec;

	prec = getRegistered(exc_id);
	if (!prec) {
		fprintf(stderr, FMT_ERR("SyncP_1 (Pre-Change) EXC [%d] FAILED "
				"(Error: Execution Context not registered)\n"),
				exc_id);
		return RTLIB_EXC_NOT_REGISTERED;
	}

	result = SyncP_PreChangeNotify(prec);

	// Update the Synchronziation Latency
	syncLatency = GetSyncLatency(prec);
	DB(fprintf(stderr, FMT_DBG("SyncP_1 (Pre-Change) EXC [%d], "
				"SyncLatency [%u]\n"),
				exc_id, syncLatency));

	_SyncpPrechangeResp(token, prec, syncLatency);

	return RTLIB_OK;
}


RTLIB_ExitCode BbqueRPC::Set(
		const RTLIB_ExecutionContextHandler ech,
		RTLIB_Constraint* constraints,
		uint8_t count) {
	//Silence "args not used" warning.
	(void)ech;
	(void)constraints;
	(void)count;

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC::Clear(
		const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC::StopExecution(
		RTLIB_ExecutionContextHandler ech,
		struct timespec timeout) {
	//Silence "args not used" warning.
	(void)ech;
	(void)timeout;

	return RTLIB_OK;
}

} // namespace rtlib

} // namespace bbque

