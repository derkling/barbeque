/**
 *       @file  bbque_exc.cc
 *      @brief  The base class for EXC being managed by the RTLIB
 *
 * This is a base class suitable for the implementation of EXC that should be
 * managed by the Barbeque RTRM.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  04/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "bbque/rtlib/bbque_exc.h"

#include "bbque/utils/utility.h"

#include <assert.h>

#ifdef BBQUE_DEBUG
# warning Debugging is enabled
#endif

#ifndef BBQUE_RTLIB_DEFAULT_CYCLES
# define BBQUE_RTLIB_DEFAULT_CYCLES 8
#endif

#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "RTLIB_EXC  [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "RTLIB_EXC  [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "RTLIB_EXC  [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "RTLIB_EXC  [ERR]", fmt)

namespace bbque { namespace rtlib {

BbqueEXC::BbqueEXC(std::string const & name,
		std::string const & recipe,
		RTLIB_Services_t * const rtl) :
	exc_name(name), rpc_name(recipe), rtlib(rtl), cycles_count(0),
	registered(false), started(false), enabled(false), done(false) {

	RTLIB_ExecutionContextParams_t exc_params = {
		{RTLIB_VERSION_MAJOR, RTLIB_VERSION_MINOR},
		RTLIB_LANG_CPP,
		recipe.c_str()
	};

	assert(rtlib);

	fprintf(stderr, FMT_INF("Initializing a new EXC [%s]...\n"),
			name.c_str());

	//--- Register
	assert(rtlib->Register);
	exc_hdl = rtlib->Register(name.c_str(), &exc_params);
	if (!exc_hdl) {
		fprintf(stderr, FMT_ERR("Registering EXC [%s] FAILED\n"),
				name.c_str());
		// TODO set initialization not completed
		return;
	}
	registered = true;

	//--- Spawn the control thread
	ctrl_trd = std::thread(&BbqueEXC::ControlLoop, this);

}


BbqueEXC::~BbqueEXC() {
	//--- Disable the EXC and the Control-Loop
	Disable();
	//--- Unregister the EXC and Terminate the control loop thread
	Terminate();
}


/*******************************************************************************
 *    Execution Context Management
 ******************************************************************************/

RTLIB_ExitCode_t BbqueEXC::_Enable() {
	RTLIB_ExitCode_t result;

	assert(registered == true);

	if (enabled)
		return RTLIB_OK;

	fprintf(stderr, FMT_INF("Enabling EXC [%s] (@%p)...\n"),
			exc_name.c_str(), (void*)exc_hdl);

	assert(rtlib->Enable);
	result = rtlib->Enable(exc_hdl);
	if (result != RTLIB_OK) {
		fprintf(stderr, FMT_ERR("Enabling EXC [%s] (@%p) FAILED\n"),
				exc_name.c_str(), (void*)exc_hdl);
		return result;
	}

	//--- Mark the control loop as Enabled
	// NOTE however, the thread should be started only with an actual call to
	// the Start() method
	enabled = true;

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueEXC::Enable() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);
	if (!started)
		return RTLIB_EXC_NOT_STARTED;
	return _Enable();
}

RTLIB_ExitCode_t BbqueEXC::Disable() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);
	RTLIB_ExitCode_t result;

	assert(registered == true);

	if (!enabled)
		return RTLIB_OK;

	fprintf(stderr, FMT_INF("Disabling control loop for EXC [%s] (@%p)...\n"),
			exc_name.c_str(), (void*)exc_hdl);

	//--- Notify the control-thread we are STOPPED
	enabled = false;
	ctrl_cv.notify_all();

	//--- Disable the EXC
	fprintf(stderr, FMT_INF("Disabling EXC [%s] (@%p)...\n"),
			exc_name.c_str(), (void*)exc_hdl);

	assert(rtlib->Disable);
	result = rtlib->Disable(exc_hdl);

	return result;

}

RTLIB_ExitCode_t BbqueEXC::Start() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);
	RTLIB_ExitCode_t result;

	assert(registered == true);

	//--- Enable the working mode to get resources
	result = _Enable();
	if (result != RTLIB_OK)
		return result;

	//--- Notify the control-thread we are STARTED
	started = true;
	ctrl_cv.notify_all();

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueEXC::Terminate() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);

	// Check if we are already terminating
	if (!registered)
		return RTLIB_OK;

	// Unregister the EXC
	fprintf(stderr, FMT_INF("Unregistering EXC [%s] (@%p)...\n"),
			exc_name.c_str(), (void*)exc_hdl);

	assert(rtlib->Unregister);
	rtlib->Unregister(exc_hdl);

	// Check if the control loop has already terminated
	if (done) {
		// Notify the WaitCompletion before exiting
		ctrl_trd.join();
		// Joining the terminated thread (for a clean exit)
		ctrl_cv.notify_all();
		return RTLIB_OK;
	}

	fprintf(stderr, FMT_INF("Terminating control loop for EXC [%s] (@%p)...\n"),
			exc_name.c_str(), (void*)exc_hdl);

	// Notify the control thread we are done
	done = true;
	ctrl_cv.notify_all();
	ctrl_ul.unlock();

	// Wait for the control thread to finish
	ctrl_trd.join();

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueEXC::WaitCompletion() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);

	fprintf(stderr, FMT_INF("Waiting for EXC [%s] control "
				"loop termination...\n"), exc_name.c_str());

	while (!done)
		ctrl_cv.wait(ctrl_ul);

	return RTLIB_OK;
}

/*******************************************************************************
 *    Default Events Handler
 ******************************************************************************/

RTLIB_ExitCode_t BbqueEXC::onSetup() {

	DB(fprintf(stderr, FMT_WRN("<< Default setup of EXC [%s]  >>\n"),
				exc_name.c_str()));
	DB(::usleep(10000));

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueEXC::onConfigure(uint8_t awm_id) {

	DB(fprintf(stderr, FMT_WRN("<< Default switching of EXC [%s] "
					"into AWM [%d], latency 10[ms] >>\n"),
				exc_name.c_str(), awm_id));
	DB(::usleep(10000));

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueEXC::onSuspend() {

	DB(fprintf(stderr, FMT_WRN("<< Default suspending of EXC [%s],"
					"latency 10[ms] >>\n"),
				exc_name.c_str()));
	DB(::usleep(10000));

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueEXC::onRun() {

	// By default return after a pre-defined number of cycles
	if (cycles_count >= BBQUE_RTLIB_DEFAULT_CYCLES)
		return RTLIB_EXC_WORKLOAD_NONE;

	DB(fprintf(stderr, FMT_WRN("<< Default onRun: EXC [%s], AWM[%02d], "
					"cycle [%d/%d], latency %d[ms] >>\n"),
				exc_name.c_str(), wmp.awm_id,
				cycles_count+1, BBQUE_RTLIB_DEFAULT_CYCLES,
				100*(wmp.awm_id+1)));
	DB(::usleep((wmp.awm_id+1)*100000));

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueEXC::onMonitor() {

	DB(fprintf(stderr, FMT_WRN("<< Default monitoring of EXC [%s],"
					"latency 1[ms] >>\n"),
				exc_name.c_str()));
	DB(::usleep(1000));

	return RTLIB_OK;
}


/*******************************************************************************
 *    Constraints Management
 ******************************************************************************/

RTLIB_ExitCode_t BbqueEXC::SetConstraints(
		RTLIB_Constraint_t *constraints,
		uint8_t count) {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);
	RTLIB_ExitCode_t result = RTLIB_OK;

	assert(registered == true);
	assert(rtlib->SetConstraints);

	//--- Assert constraints on this EXC
	fprintf(stderr, FMT_INF("Set [%d] constraints for EXC [%s] (@%p)...\n"),
			count, exc_name.c_str(), (void*)exc_hdl);
	result = rtlib->SetConstraints(exc_hdl, constraints, count);

	return result;
}

RTLIB_ExitCode_t BbqueEXC::ClearConstraints() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);
	RTLIB_ExitCode_t result = RTLIB_OK;

	assert(registered == true);
	assert(rtlib->ClearConstraints);

	//--- Clear constraints on this EXC
	fprintf(stderr, FMT_INF("Clear ALL constraints for EXC [%s] (@%p)...\n"),
			exc_name.c_str(), (void*)exc_hdl);
	result = rtlib->ClearConstraints(exc_hdl);

	return result;
}


/*******************************************************************************
 *    Control Loop
 ******************************************************************************/

RTLIB_ExitCode_t BbqueEXC::Setup() {
	RTLIB_ExitCode_t result;

	DB(fprintf(stderr, FMT_DBG("CL 0. Setup EXC [%s]...\n"),
			exc_name.c_str()));

	result = onSetup();
	return result;
}

bool BbqueEXC::WaitEnable() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);

	while (!done && !enabled)
		ctrl_cv.wait(ctrl_ul);

	return done;
}

RTLIB_ExitCode_t BbqueEXC::GetWorkingMode() {
	RTLIB_ExitCode_t result;

	DB(fprintf(stderr, FMT_DBG("CL 1. Get AWM for EXC [%s]...\n"),
			exc_name.c_str()));

	assert(rtlib->GetWorkingMode);
	result = rtlib->GetWorkingMode(exc_hdl, &wmp, RTLIB_SYNC_STATELESS);

	return result;
}

RTLIB_ExitCode_t BbqueEXC::Reconfigure(RTLIB_ExitCode_t result) {

	DB(fprintf(stderr, FMT_DBG("CL 2. Reconfigure check for EXC [%s]...\n"),
				exc_name.c_str()));

	switch (result) {
	case RTLIB_OK:
		DB(fprintf(stderr, FMT_DBG("CL 2-1. Continue to run "
				"on the assigned AWM [%d] for EXC [%s]\n"),
				wmp.awm_id, exc_name.c_str()));
		return result;

	case RTLIB_EXC_GWM_START:
	case RTLIB_EXC_GWM_RECONF:
	case RTLIB_EXC_GWM_MIGREC:
	case RTLIB_EXC_GWM_MIGRATE:
		DB(fprintf(stderr, FMT_DBG("CL 2-2. Switching EXC [%s] "
				"to AWM [%02d]...\n"),
				exc_name.c_str(), wmp.awm_id));

		rtlib->Notify.PreConfigure(exc_hdl);
		onConfigure(wmp.awm_id);
		rtlib->Notify.PostConfigure(exc_hdl);
		return result;

	case RTLIB_EXC_GWM_BLOCKED:
		DB(fprintf(stderr, FMT_DBG("CL 2-3. Suspending EXC [%s]...\n"),
				exc_name.c_str()));
		rtlib->Notify.PreSuspend(exc_hdl);
		onSuspend();
		rtlib->Notify.PostSuspend(exc_hdl);
		return result;

	default:
		DB(fprintf(stderr, FMT_ERR("GetWorkingMode for EXC [%s] FAILED "
					"(Error: Invalid event [%d])\n"),
					exc_name.c_str(), result));
		assert(result >= RTLIB_EXC_GWM_START);
		assert(result <= RTLIB_EXC_GWM_BLOCKED);
	}

	return RTLIB_EXC_GWM_FAILED;
}


RTLIB_ExitCode_t BbqueEXC::Run() {
	RTLIB_ExitCode_t result;

	DB(fprintf(stderr, FMT_DBG("CL 3. Run EXC [%s], cycle [%010d], "
					"AWM[%02d]...\n"),
				exc_name.c_str(), cycles_count+1, wmp.awm_id));

	rtlib->Notify.PreRun(exc_hdl);

	result = onRun();
	if (result == RTLIB_EXC_WORKLOAD_NONE) {
		done = true;
	} else {
		rtlib->Notify.PostRun(exc_hdl);
	}

	return result;
}

RTLIB_ExitCode_t BbqueEXC::Monitor() {
	RTLIB_ExitCode_t result;

	// Account executed cycles
	cycles_count++;

	DB(fprintf(stderr, FMT_DBG("CL 4. Monitor EXC [%s]...\n"),
				exc_name.c_str()));

	rtlib->Notify.PreMonitor(exc_hdl);
	result = onMonitor();
	rtlib->Notify.PostMonitor(exc_hdl);
	if (result == RTLIB_EXC_WORKLOAD_NONE)
		done = true;

	return result;
}

void BbqueEXC::ControlLoop() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);
	RTLIB_ExitCode_t result;

	assert(rtlib);
	assert(registered == true);

	// Wait for the EXC being STARTED
	while (!started)
		ctrl_cv.wait(ctrl_ul);
	ctrl_ul.unlock();

	DB(fprintf(stderr, FMT_DBG("EXC [%s] control thread [%d] started...\n"),
				exc_name.c_str(), gettid()));

	assert(enabled == true);

	// Setup notification
	rtlib->Notify.Setup(exc_hdl);

	// Setup the EXC
	Setup();

	// Initialize notification
	rtlib->Notify.Init(exc_hdl);

	// Endless loop
	while (!done) {

		// Wait for the EXC being enabled
		if (WaitEnable())
			break;

		// Get the assigned AWM
		result = GetWorkingMode();
		if (result == RTLIB_EXC_GWM_FAILED)
			continue;

		// Check for reconfiguration required
		result = Reconfigure(result);
		if (result != RTLIB_OK)
			continue;

		// Run the workload
		result = Run();
		if (done || result != RTLIB_OK)
			continue;

		// Monitor Quality-of-Services
		result = Monitor();
		if (done || result != RTLIB_OK)
			continue;

	};

	// Disable the EXC (thus notifying waiters)
	Disable();

	// Exit notification
	rtlib->Notify.Exit(exc_hdl);

	DB(fprintf(stderr, FMT_ERR("Control-loop for EXC [%s] TERMINATED\n"),
				exc_name.c_str()));

}

} // namespace rtlib

} // namespace bbque
