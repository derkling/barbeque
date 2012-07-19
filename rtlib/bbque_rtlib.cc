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

#include "bbque/rtlib.h"

#include "bbque/version.h"
#include "bbque/utils/timer.h"
#include "bbque/utils/utility.h"

#include "bbque/rtlib/bbque_rpc.h"

#ifdef CONFIG_TARGET_ANDROID
# include <stdint.h>
#else
# include <cstdint>
#endif

namespace bb = bbque;
namespace bu = bbque::utils;
namespace br = bbque::rtlib;

// Setup logging
#undef  BBQUE_LOG_MODULE
#define BBQUE_LOG_MODULE "rtl"

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

static RTLIB_ExitCode_t rtlib_ggap(
		RTLIB_ExecutionContextHandler_t ech,
		uint8_t gap) {
	return rpc->GGap(ech, gap);
}

/*******************************************************************************
 *    Utility Functions
 ******************************************************************************/

static const char *rtlib_utils_getuid() {
	return rpc->GetUid();
}

/*******************************************************************************
 *    Cycles Per Second (CPS) Control Support
 ******************************************************************************/

static RTLIB_ExitCode_t rtlib_cps_set(
		RTLIB_ExecutionContextHandler_t ech,
		float cps) {
	return rpc->SetCPS(ech, cps);
}

static RTLIB_ExitCode_t rtlib_cps_set_ctime_us(
		RTLIB_ExecutionContextHandler_t ech,
		uint32_t us) {
	return rpc->SetCTimeUs(ech, us);
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

static void rtlib_notify_release(
		RTLIB_ExecutionContextHandler_t ech) {
	rpc->NotifyRelease(ech);
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
	fprintf(stderr, FI("Barbeque RTLIB (ver. %s)\n"), g_git_version);
	fprintf(stderr, FI("Built: " __DATE__  " " __TIME__ "\n"));

	// Data structure initialization
	rtlib_services.version.major = RTLIB_VERSION_MAJOR;
	rtlib_services.version.minor = RTLIB_VERSION_MINOR;

	rtlib_services.Register = rtlib_register;
	rtlib_services.Enable = rtlib_enable;
	rtlib_services.GetWorkingMode = rtlib_getwm;
	rtlib_services.SetConstraints = rtlib_set;
	rtlib_services.ClearConstraints = rtlib_clear;
	rtlib_services.SetGoalGap = rtlib_ggap;
	rtlib_services.Disable = rtlib_disable;
	rtlib_services.Unregister = rtlib_unregister;

	// Utility functions interface
	rtlib_services.Utils.GetUid = rtlib_utils_getuid;

	// Cycles Time Control interface
	rtlib_services.CPS.Set = rtlib_cps_set;
	rtlib_services.CPS.SetCTimeUs = rtlib_cps_set_ctime_us;

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
	rtlib_services.Notify.Release = rtlib_notify_release;

	// Building a communication channel
	rpc = br::BbqueRPC::GetInstance();
	if (!rpc) {
		fprintf(stderr, FE("RPC communication channel build FAILED\n"));
		return RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
	}

	// Initializing the RPC communication channel
	result = rpc->Init(name);
	if (result!=RTLIB_OK) {
		fprintf(stderr, FE("RPC communication channel "
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

	DB(fprintf(stderr, FD("Barbeque RTLIB, Cleanup and release\n")));

	if (!rtlib_initialized)
		return;

	// Close the RPC FIFO channel thus releasin all BBQUE resource used by
	// this application
	assert(rpc);

	// Ensure all the EXCs are unregistered
	rpc->UnregisterAll();
	delete rpc;

}


