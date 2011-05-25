/**
 *       @file  bbque_rtlib.cc
 *      @brief  The Run-Time Applications Library to interact with the RTRM
 *      protorype implementation for 2PARMA EU FP7 project
 *
 * This library provides an implementation of the services the Barbeque RTRM
 * exposes to applications.
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

#include "bbque/rtlib.h"

#include "bbque/version.h"
#include "bbque/utils/timer.h"
#include "bbque/utils/utility.h"

#include "bbque/rtlib/bbque_rpc.h"

namespace bb = bbque;
namespace bu = bbque::utils;
namespace br = bbque::rtlib;

#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "RTLIB      [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "RTLIB      [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "RTLIB      [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "RTLIB      [ERR]", fmt)

/**
 * The global timer, this can be used to get the time since the RTLib has been
 * initialized */
bu::Timer bbque_tmr(true);

/**
 * A pointer to the Barbeque RPC communication channel
 */
static br::BbqueRPC *rpc = NULL;

/**
 * The collection of RTLib services accessible from applications.
 */
static RTLIB_Services rtlib_services;

static RTLIB_ExecutionContextHandler rtlib_register(const char *name,
		const RTLIB_ExecutionContextParams *params) {
	return rpc->Register(name, params);
}

static void rtlib_unregister(
		const RTLIB_ExecutionContextHandler ech) {
	return rpc->Unregister(ech);
}

static RTLIB_ExitCode rtlib_start(
		const RTLIB_ExecutionContextHandler ech) {
	return rpc->Start(ech);
}

static RTLIB_ExitCode rtlib_stop(
		const RTLIB_ExecutionContextHandler ech) {
	return rpc->Stop(ech);
}

static bool rtlib_sync(
		const RTLIB_ExecutionContextHandler ech,
		const char *name, RTLIB_SyncType type) {
	return rpc->Sync(ech, name, type);
}

static RTLIB_ExitCode rtlib_getwm(
		const RTLIB_ExecutionContextHandler ech,
		RTLIB_WorkingModeParams *wm) {
	return rpc->GetWorkingMode(ech, wm);
}

static RTLIB_ExitCode rtlib_set(
		RTLIB_ExecutionContextHandler ech,
		RTLIB_Constraint *constraints, uint8_t count) {
	return rpc->Set(ech, constraints, count);
}

static RTLIB_ExitCode rtlib_clear(
		RTLIB_ExecutionContextHandler ech) {
	return rpc->Clear(ech);
}

static const char *rtlib_app_name;
static uint8_t rtlib_initialized = 0;

RTLIB_Services *RTLIB_Init(const char *name) {
	RTLIB_ExitCode result;

	assert(rtlib_initialized==0);

	// Welcome screen
	fprintf(stderr, FMT_INF("Barbeque RTLIB (ver. %s)\n"), g_git_version);
	fprintf(stderr, FMT_INF("Built: " __DATE__  " " __TIME__ "\n"));

	// Data structure initialization
	rtlib_services.version.major = RTLIB_VERSION_MAJOR;
	rtlib_services.version.minor = RTLIB_VERSION_MINOR;
	rtlib_services.RegisterExecutionContext = rtlib_register;
	rtlib_services.StartExecutionContext = rtlib_start;
	rtlib_services.NotifySync = rtlib_sync;
	rtlib_services.GetWorkingMode = rtlib_getwm;
	rtlib_services.SetConstraints = rtlib_set;
	rtlib_services.ClearConstraints = rtlib_clear;
	rtlib_services.StopExecutionContext = rtlib_stop;
	rtlib_services.UnregisterExecutionContext = rtlib_unregister;

	// Building a communication channel
	rpc = br::BbqueRPC::GetInstance();
	if (!rpc) {
		fprintf(stderr, FMT_ERR("RPC communication channel build FAILED\n"));
		return NULL;
	}

	// Initializing the RPC communication channel
	result = rpc->Init(name);
	if (result!=RTLIB_OK) {
		fprintf(stderr, FMT_ERR("RPC communication channel "
					"initialization FAILED\n"));
		return NULL;
	}

	// Marking library as intialized
	rtlib_initialized = 1;
	rtlib_app_name = name;

	return &rtlib_services;
}

__attribute__((destructor))
static void RTLIB_Exit(void) {

	DB(fprintf(stderr, FMT_DBG("Barbeque RTLIB Destructor\n")));

	if (!rtlib_initialized)
		return;

	// Close the RPC FIFO channel thus releasin all BBQUE resource used by
	// this application
	assert(rpc);
	delete rpc;

}

