/**
 *       @file  bbque_exc.cc
 *      @brief  The base class for EXC being managed by the RTLIB
 *
 * This is a base class suitable for the implementation of EXC that should be
 * managed by the Barbeque RTRM.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
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

#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "RTLIB_EXC  [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "RTLIB_EXC  [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "RTLIB_EXC  [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "RTLIB_EXC  [ERR]", fmt)

BbqueEXC::BbqueEXC(std::string const & name,
		std::string const & recipe,
		RTLIB_Services_t * const rtl,
		bool enabled) :
	exc_name(name), rpc_name(recipe), rtlib(rtl),
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

	//--- Enable the working mode (if required)
	if (enabled) {
		Enable();
	}

	//--- Spawn the control thread
	ctrl_trd = std::thread(&BbqueEXC::ControlLoop, this);

}


BbqueEXC::~BbqueEXC() {
	RTLIB_ExitCode_t result;

	//--- Disable the Control-Loop

	Disable();

	//--- Disable the EXC

	fprintf(stderr, FMT_INF("Disabling EXC [%s] (@%p)...\n"),
			exc_name.c_str(), (void*)exc_hdl);

	assert(rtlib->Disable);
	result = rtlib->Disable(exc_hdl);

	//--- Unregister the EXC

	fprintf(stderr, FMT_INF("Unregistering EXC [%s] (@%p)...\n"),
			exc_name.c_str(), (void*)exc_hdl);

	assert(rtlib->Unregister);
	rtlib->Unregister(exc_hdl);

	//--- Terminate the control loop thread
	
	Terminate();

}


/*******************************************************************************
 *    Execution Context Management
 ******************************************************************************/

RTLIB_ExitCode_t BbqueEXC::Enable() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);
	RTLIB_ExitCode_t result;

	assert(registered == true);

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

RTLIB_ExitCode_t BbqueEXC::Disable() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);

	assert(registered == true);

	if (!enabled)
		return RTLIB_OK;

	fprintf(stderr, FMT_INF("Disabling control loop for EXC [%s] (@%p)...\n"),
			exc_name.c_str(), (void*)exc_hdl);

	//--- Notify the control-thread we are STOPPED
	enabled = false;
	ctrl_cv.notify_all();

	return RTLIB_OK;

}

RTLIB_ExitCode_t BbqueEXC::Start() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);

	assert(registered == true);

	if (!enabled)
		return RTLIB_EXC_NOT_ENABLED;

	//--- Notify the control-thread we are STARTED

	started = true;
	ctrl_cv.notify_all();

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueEXC::Terminate() {
	std::unique_lock<std::mutex> ctrl_ul(ctrl_mtx);

	// Check if we are already terminating
	if (done || !registered)
		return RTLIB_OK;

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


/*******************************************************************************
 *    Default Events Handler
 ******************************************************************************/

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

	DB(fprintf(stderr, FMT_WRN("<< Default running of EXC [%s],"
					"latency 100[sm] >>\n"),
				exc_name.c_str()));
	DB(::usleep(100000));

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
 *    Control Loop
 ******************************************************************************/

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
				"to AWM [%d]...\n"),
				exc_name.c_str(), wmp.awm_id));
		onConfigure(wmp.awm_id);
		return result;

	case RTLIB_EXC_GWM_BLOCKED:
		DB(fprintf(stderr, FMT_DBG("CL 2-3. Suspending EXC [%s]...\n"),
				exc_name.c_str()));
		onSuspend();
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

	DB(fprintf(stderr, FMT_DBG("CL 3. Run EXC [%s]...\n"),
				exc_name.c_str()));

	result = onRun();

	return result;
}

RTLIB_ExitCode_t BbqueEXC::Monitor() {
	RTLIB_ExitCode_t result;

	DB(fprintf(stderr, FMT_DBG("CL 4. Monitor EXC [%s]...\n"),
				exc_name.c_str()));

	result = onMonitor();

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

	assert(enabled == true);

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
		if (result != RTLIB_OK)
			continue;

		// Monitor Quality-of-Services
		result = Monitor();
		if (result != RTLIB_OK)
			continue;

	};

	DB(fprintf(stderr, FMT_DBG("Control-loop for EXC [%s] TERMINATED\n"),
				exc_name.c_str()));

}

