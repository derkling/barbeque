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

#define FMT_DBG(fmt) "RTLIB [DBG] - "fmt
#define FMT_INF(fmt) "RTLIB [INF] - "fmt
#define FMT_WRN(fmt) "RTLIB [WRN] - "fmt
#define FMT_ERR(fmt) "RTLIB [ERR] - "fmt

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

BbqueRPC::BbqueRPC(void) {

}

BbqueRPC::~BbqueRPC(void) {
	DB(fprintf(stderr, FMT_DBG("Releaseing the RPC channel...\n")));
}

RTLIB_ExitCode BbqueRPC::Init(const char *name) {
	RTLIB_ExitCode exitCode;

	DB(fprintf(stderr, FMT_DBG("Initializing app [%s]\n"), name));

	exitCode = _Init(name);
	if (exitCode != RTLIB_OK) {
		fprintf(stderr, FMT_ERR("Initialization FAILED\n"));
		return exitCode;
	}

	DB(fprintf(stderr, FMT_DBG("Initialation DONE\n")));
	return RTLIB_OK;

}

RTLIB_ExecutionContextHandler BbqueRPC::Register(
		const char* name,
		const RTLIB_ExecutionContextParams* params) {
	//Silence "args not used" warning.
	(void)name;
	(void)params;

	return NULL;
}

void BbqueRPC::Unregister(
		const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;

}

RTLIB_ExitCode BbqueRPC::Start(
		const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC::Stop(
		const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;

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

RTLIB_ExitCode BbqueRPC::GetWorkingMode(
			RTLIB_ExecutionContextHandler ech,
			RTLIB_WorkingModeParams *wm) {
	//Silence "args not used" warning.
	(void)ech;
	(void)wm;


	return RTLIB_NO_WORKING_MODE;
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

