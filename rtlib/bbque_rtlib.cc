/**
 *       @file  bbque_rtlib.cc
 *      @brief  The Run-Time Applications Library to interact with the RTRM
 *      protorype implementation for 2PARMA EU FP7 project
 *
 * This library provides an implementation of the services the Barbeque RTRM
 * exposes to applications.
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
static RTLIB_Services_t rtlib_services;

static RTLIB_ExecutionContextHandler_t rtlib_register(const char *name,
		const RTLIB_ExecutionContextParams_t *params) {
	return rpc->Register(name, params);
}

static void rtlib_unregister(
		const RTLIB_ExecutionContextHandler_t ech) {
	return rpc->Unregister(ech);
}

static RTLIB_ExitCode_t rtlib_enable(
		const RTLIB_ExecutionContextHandler_t ech) {
	return rpc->Enable(ech);
}

static RTLIB_ExitCode_t rtlib_disable(
		const RTLIB_ExecutionContextHandler_t ech) {
	return rpc->Disable(ech);
}

static RTLIB_ExitCode_t rtlib_getwm(
		const RTLIB_ExecutionContextHandler_t ech,
		RTLIB_WorkingModeParams_t *wm,
		RTLIB_SyncType_t st) {
	return rpc->GetWorkingMode(ech, wm, st);
}

static RTLIB_ExitCode_t rtlib_set(
		RTLIB_ExecutionContextHandler_t ech,
		RTLIB_Constraint_t *constraints, uint8_t count) {
	return rpc->Set(ech, constraints, count);
}

static RTLIB_ExitCode_t rtlib_clear(
		RTLIB_ExecutionContextHandler_t ech) {
	return rpc->Clear(ech);
}


/*******************************************************************************
 *    Performance Monitoring Support
 ******************************************************************************/

static void rtlib_notify_setup(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifySetup(ech);
}

static void rtlib_notify_init(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyInit(ech);
}

static void rtlib_notify_exit(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyExit(ech);
}

static void rtlib_notify_pre_configure(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPreConfigure(ech);
}

static void rtlib_notify_post_configure(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPostConfigure(ech);
}

static void rtlib_notify_pre_run(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPreRun(ech);
}

static void rtlib_notify_post_run(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPostRun(ech);
}

static void rtlib_notify_pre_monitor(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPreMonitor(ech);
}

static void rtlib_notify_post_monitor(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPostMonitor(ech);
}

static void rtlib_notify_pre_suspend(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPreSuspend(ech);
}

static void rtlib_notify_post_suspend(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPostSuspend(ech);
}

static void rtlib_notify_pre_resume(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPreResume(ech);
}

static void rtlib_notify_post_resume(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyPostResume(ech);
}

static const char *rtlib_app_name;
static uint8_t rtlib_initialized = 0;

#include "bbque_errors.cc"

RTLIB_ExitCode_t RTLIB_Init(const char *name, RTLIB_Services_t **rtlib) {
	RTLIB_ExitCode_t result;
	(*rtlib) = NULL;

	// Checking error string array consistency
	static_assert(ARRAY_SIZE(RTLIB_errorStr) == RTLIB_EXIT_CODE_COUNT,
			"RTLIB error strings not matching errors count");

	assert(rtlib_initialized==0);

	// Welcome screen
	fprintf(stderr, FMT_INF("Barbeque RTLIB (ver. %s)\n"), g_git_version);
	fprintf(stderr, FMT_INF("Built: " __DATE__  " " __TIME__ "\n"));

	// Data structure initialization
	rtlib_services.version.major = RTLIB_VERSION_MAJOR;
	rtlib_services.version.minor = RTLIB_VERSION_MINOR;

	rtlib_services.Register = rtlib_register;
	rtlib_services.Enable = rtlib_enable;
	rtlib_services.GetWorkingMode = rtlib_getwm;
	rtlib_services.SetConstraints = rtlib_set;
	rtlib_services.ClearConstraints = rtlib_clear;
	rtlib_services.Disable = rtlib_disable;
	rtlib_services.Unregister = rtlib_unregister;

	// Performance monitoring notifiers
	rtlib_services.Notify.Setup = rtlib_notify_setup;
	rtlib_services.Notify.Init = rtlib_notify_init;
	rtlib_services.Notify.Exit = rtlib_notify_exit;
	rtlib_services.Notify.PreConfigure = rtlib_notify_pre_configure;
	rtlib_services.Notify.PostConfigure = rtlib_notify_post_configure;
	rtlib_services.Notify.PreRun = rtlib_notify_pre_run;
	rtlib_services.Notify.PostRun = rtlib_notify_post_run;
	rtlib_services.Notify.PreMonitor = rtlib_notify_pre_monitor;
	rtlib_services.Notify.PostMonitor = rtlib_notify_post_monitor;
	rtlib_services.Notify.PreSuspend = rtlib_notify_pre_suspend;
	rtlib_services.Notify.PostSuspend = rtlib_notify_post_suspend;
	rtlib_services.Notify.PreResume = rtlib_notify_pre_resume;
	rtlib_services.Notify.PostResume = rtlib_notify_post_resume;

	// Building a communication channel
	rpc = br::BbqueRPC::GetInstance();
	if (!rpc) {
		fprintf(stderr, FMT_ERR("RPC communication channel build FAILED\n"));
		return RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
	}

	// Initializing the RPC communication channel
	result = rpc->Init(name);
	if (result!=RTLIB_OK) {
		fprintf(stderr, FMT_ERR("RPC communication channel "
					"initialization FAILED\n"));
		return RTLIB_BBQUE_UNREACHABLE;
	}

	// Marking library as intialized
	rtlib_initialized = 1;
	rtlib_app_name = name;

	(*rtlib) = &rtlib_services;
	return RTLIB_OK;
}

__attribute__((destructor))
static void RTLIB_Exit(void) {

	DB(fprintf(stderr, FMT_DBG("Barbeque RTLIB, Cleanup and release\n")));

	if (!rtlib_initialized)
		return;

	// Close the RPC FIFO channel thus releasin all BBQUE resource used by
	// this application
	assert(rpc);

	// Ensure all the EXCs are unregistered
	rpc->UnregisterAll();
	delete rpc;

}


