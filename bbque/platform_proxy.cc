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

#include "bbque/platform_proxy.h"

#include "bbque/utils/utility.h"
#include "bbque/modules_factory.h"
#include "bbque/test_platform_data.h"

# ifdef CONFIG_TARGET_LINUX
#  include "bbque/pp/linux.h"
#  define  PLATFORM_PROXY LinuxPP
# endif
# ifdef CONFIG_TARGET_P2012
#  include "bbque/pp/p2012.h"
#  define  PLATFORM_PROXY P2012PP
# endif

namespace bb = bbque;
namespace br = bbque::res;

namespace bbque {

PlatformProxy::PlatformProxy() :
	trdRunning(false),
	done(false),
	pilInitialized(false),
	platformIdentifier(NULL) {

	// Get a logger module
	std::string logger_name(PLATFORM_PROXY_NAMESPACE);
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

#ifdef BBQUE_TEST_PLATFORM_DATA
#warning Using TEST PLATFORM DATA
	return;
#endif // BBQUE_TEST_PLATFORM_DATA

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
	if (prctl(PR_SET_NAME, (long unsigned int)BBQUE_MODULE_NAME("pp"), 0, 0, 0) != 0) {
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
	ResourceAccounter &ra(ResourceAccounter::GetInstance());
	ExitCode_t result = OK;

#ifdef BBQUE_TEST_PLATFORM_DATA
	//---------- Loading TEST platform data
	logger->Debug("PLAT PRX: loading TEST PLATFORM data");
	// This is just a temporary placeholder for the (still-missing)
	// ResourceAbstraction module
	bb::TestPlatformData &tpd(bb::TestPlatformData::GetInstance());
	tpd.LoadPlatformData();
#else

	// Return if the PIL has not been properly initialized
	if (!pilInitialized) {
		logger->Fatal("PLAT PRX: Platform Integration Layer initialization FAILED");
		return PLATFORM_INIT_FAILED;
	}

	// Platform specific resources enumeration
	logger->Debug("PLAT PRX: loading platform data");
	result = _LoadPlatformData();
	if (unlikely(result != OK)) {
		logger->Fatal("PLAT PRX: Platform [%s] initialization FAILED",
				GetPlatformID());
		return result;
	}
#endif // BBQUE_TEST_PLATFORM_DATA

	// Setup the Platform Specific ID
	platformIdentifier = _GetPlatformID();

	logger->Notice("PLAT PRX: Platform [%s] initialization COMPLETED",
			GetPlatformID());

	// Dump status of registered resource
	ra.PrintStatusReport(0, true);

	return result;
}

PlatformProxy::ExitCode_t
PlatformProxy::Setup(AppPtr_t papp) {
	ExitCode_t result = OK;

	logger->Debug("PLAT PRX: platform setup for run-time control "
			"of app [%s]", papp->StrId());
#ifndef BBQUE_TEST_PLATFORM_DATA
	result = _Setup(papp);
#endif // !BBQUE_TEST_PLATFORM_DATA
	return result;
}

PlatformProxy::ExitCode_t
PlatformProxy::Release(AppPtr_t papp) {
	ExitCode_t result = OK;

	logger->Debug("PLAT PRX: releasing platform-specific run-time control "
			"for app [%s]", papp->StrId());
#ifndef BBQUE_TEST_PLATFORM_DATA
	result = _Release(papp);
#endif // !BBQUE_TEST_PLATFORM_DATA
	return result;
}

PlatformProxy::ExitCode_t
PlatformProxy::ReclaimResources(AppPtr_t papp) {
	ExitCode_t result = OK;

	logger->Debug("PLAT PRX: Reclaiming resources of app [%s]", papp->StrId());
#ifndef BBQUE_TEST_PLATFORM_DATA
	result = _ReclaimResources(papp);
#endif // !BBQUE_TEST_PLATFORM_DATA
	return result;
}

PlatformProxy::ExitCode_t
PlatformProxy::MapResources(AppPtr_t papp, UsagesMapPtr_t pres, bool excl) {
	ResourceAccounter &ra = ResourceAccounter::GetInstance();
	RViewToken_t rvt = ra.GetScheduledView();
	ExitCode_t result = OK;

	logger->Debug("PLAT PRX: Mapping resources for app [%s], using view [%d]",
			papp->StrId(), rvt);

#ifndef BBQUE_TEST_PLATFORM_DATA

	// Platform Specific Data (PSD) should be initialized the first time
	// an application is scheduled for execution
	if (unlikely(!papp->HasPlatformData())) {

		// Setup PSD
		result = Setup(papp);
		if (result != OK) {
			logger->Error("Setup PSD for EXC [%s] FAILED",
					papp->StrId());
			return result;
		}

		// Mark PSD as correctly initialized
		papp->SetPlatformData();
	}

	// Map resources
	result = _MapResources(papp, pres, rvt, excl);

#else

	// Quite compiler warnings on TEST PLATFORM MODE
	(void)pres;
	(void) excl;
#endif // !BBQUE_TEST_PLATFORM_DATA
	return result;
}

} /* bbque */
