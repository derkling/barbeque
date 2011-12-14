/**
 *       @file  platform_proxy.cc
 *      @brief  A proxy to interact with the target platform
 *
 * Implementation of the PlatformProxy class
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  11/23/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "bbque/platform_proxy.h"

#include "bbque/utils/utility.h"
#include "bbque/modules_factory.h"
#include "bbque/test_platform_data.h"

#if defined BBQUE_PLATFORM_LINUX
# include "bbque/pp/linux.h"
# define  PLATFORM_PROXY LinuxPP
#elif defined BBQUE_PLATFORM_P2012
# include "bbque/pp/p2012.h"
# define  PLATFORM_PROXY P2012PP
#endif

namespace bb = bbque;
namespace br = bbque::res;

namespace bbque {

PlatformProxy::PlatformProxy() :
	trdRunning(false),
	done(false) {

	// Get a logger module
	std::string logger_name(PLATFORM_PROXY_NAMESPACE);
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

	// Spawn the platform monitoring thread
	monitor_thd = std::thread(&PlatformProxy::Monitor, this);

}

PlatformProxy::~PlatformProxy() {
	// Stopping the monitor thread
	Stop();
}

PlatformProxy & PlatformProxy::GetInstance() {
	static PLATFORM_PROXY instance;
	return instance;
}


void PlatformProxy::Start() {
	std::unique_lock<std::mutex> ul(trdStatus_mtx);

	logger->Debug("PLAT PRX: starting the monitoring service...");
	trdRunning = true;
	trdStatus_cv.notify_one();
}

void PlatformProxy::Stop() {
	std::unique_lock<std::mutex> ul(trdStatus_mtx);

	if (done == true)
		return;

	logger->Debug("PLAT PRX: stopping the monitoring service...");
	done = true;
	trdStatus_cv.notify_one();
}

void PlatformProxy::Monitor() {
	std::unique_lock<std::mutex> trdStatus_ul(trdStatus_mtx);

	// Set the module name
	if (prctl(PR_SET_NAME, BBQUE_MODULE_NAME("pp"), 0, 0, 0) != 0) {
		logger->Error("Set name FAILED! (Error: %s)\n", strerror(errno));
	}

	// Waiting for thread authorization to start
	if (!trdRunning)
		trdStatus_cv.wait(trdStatus_ul);

	trdStatus_ul.unlock();

	logger->Info("PLAT PRX: Monitoring thread STARTED");

	while (!done) {
		// TODO place here the code to monitor for resources availability and
		// run-time status (e.g. thermal, load-average)
		trdStatus_ul.lock();
		trdStatus_cv.wait(trdStatus_ul);
	}

	logger->Info("PLAT PRX: Monitoring thread ENDED");
}


PlatformProxy::ExitCode_t
PlatformProxy::LoadPlatformData() {
	ExitCode_t result = OK;

#if BBQUE_TEST_PLATFORM_DATA
	//---------- Loading TEST platform data
	logger->Debug("PLAT PRX: loading TEST PLATFORM data");
	// This is just a temporary placeholder for the (still-missing)
	// ResourceAbstraction module
	bb::TestPlatformData &tpd(bb::TestPlatformData::GetInstance());
	tpd.LoadPlatformData();
	return OK;
#endif // BBQUE_TEST_PLATFORM_DATA

	// Platform specific resources enumeration
	logger->Debug("PLAT PRX: loading platform data");
	result = _LoadPlatformData();
	return result;
}

PlatformProxy::ExitCode_t
PlatformProxy::Setup(AppPtr_t papp) {
	ExitCode_t result = OK;

	logger->Debug("PLAT PRX: platform setup for run-time control "
			"of app [%s]", papp->StrId());
	result = _Setup(papp);
	return result;
}

PlatformProxy::ExitCode_t
PlatformProxy::Release(AppPtr_t papp) {
	ExitCode_t result = OK;

	logger->Debug("PLAT PRX: releasing platform-specific run-time control "
			"for app [%s]", papp->StrId());
	result = _Release(papp);
	return result;
}

PlatformProxy::ExitCode_t
PlatformProxy::ReclaimResources(AppPtr_t papp) {
	ExitCode_t result = OK;

	logger->Debug("PLAT PRX: Reclaiming resources of app [%s]", papp->StrId());
	result = _ReclaimResources(papp);
	return result;
}

PlatformProxy::ExitCode_t
PlatformProxy::MapResources(AppPtr_t papp, UsagesMapPtr_t pres, bool excl) {
	br::ResourceAccounter &ra = br::ResourceAccounter::GetInstance();
	RViewToken_t rvt = ra.GetScheduledView();
	ExitCode_t result = OK;

	logger->Debug("PLAT PRX: Mapping resources for app [%s], using view [%d]",
			papp->StrId(), rvt);
	result = _MapResources(papp, pres, rvt, excl);
	return result;
}

} /* bbque */
