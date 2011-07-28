/**
 *       @file  resource_manager.cc
 *      @brief  The Barbeque Run-Time Resource Manager
 *
 * This class provides the implementation of the Run-Time Resource Manager
 * (RTRM), which is the main barbeque module implementing its glue code.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  02/01/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "bbque/resource_manager.h"

#include "bbque/configuration_manager.h"
#include "bbque/plugin_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/signals_manager.h"
#include "bbque/application_manager.h"

#include "bbque/utils/utility.h"
#include "bbque/utils/timer.h"

namespace br = bbque::res;
namespace bu = bbque::utils;
namespace bp = bbque::plugins;
namespace po = boost::program_options;

namespace bbque {

ResourceManager & ResourceManager::GetInstance() {
	static ResourceManager rtrm;

	return rtrm;
}

ResourceManager::ResourceManager() :
	ps(PlatformServices::GetInstance()),
	sm(SchedulerManager::GetInstance()),
	ym(SynchronizationManager::GetInstance()),
	am(ApplicationManager::GetInstance()),
	ap(ApplicationProxy::GetInstance()),
	pm(bp::PluginManager::GetInstance()),
	ra(br::ResourceAccounter::GetInstance()) {

}

ResourceManager::~ResourceManager() {
}

void ResourceManager::Setup() {

	//---------- Get a logger module
	std::string logger_name("bq.rm");
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "RM: Logger module creation FAILED\n");
		assert(logger);
	}

	//---------- Dump list of registered plugins
	const bp::PluginManager::RegistrationMap & rm = pm.GetRegistrationMap();
	logger->Info("RM: Registered plugins:");
	bp::PluginManager::RegistrationMap::const_iterator i;
	for (i = rm.begin(); i != rm.end(); ++i)
		logger->Info(" * %s", (*i).first.c_str());

	//---------- Start bbque services
	ap.Start();
}

void ResourceManager::NotifyEvent(controlEvent_t evt) {
	std::unique_lock<std::mutex> pendingEvts_ul(pendingEvts_mtx, std::defer_lock);

	// Ensure we have a valid event
	assert(evt<EVENTS_COUNT);

	// Set the corresponding event flag
	pendingEvts.set(evt);

	// Notify the control loop (just if it is sleeping)
	if (pendingEvts_ul.try_lock())
		pendingEvts_cv.notify_one();
}

void ResourceManager::Optimize() {
	SynchronizationManager::ExitCode_t syncResult;
	SchedulerManager::ExitCode_t schedResult;
	bu::Timer optimization_tmr;

	// Check if there is at least one application to synchronize
	if ((!am.HasApplications(Application::READY)) &&
			(!am.HasApplications(Application::RUNNING))) {
		logger->Debug("NO active EXCs, re-scheduling not required");
		return;
	}

	am.PrintStatusReport();
	ra.PrintStatusReport();
	logger->Info("Running Optimization...");

	//--- Scheduling
	logger->Debug("====================[ SCHEDULE START ]====================");
	optimization_tmr.start();
	schedResult = sm.Schedule();
	optimization_tmr.stop();
	switch(schedResult) {
	case SchedulerManager::MISSING_POLICY:
	case SchedulerManager::FAILED:
		logger->Error("EXC start FAILED (Error: scheduling policy failed)");
		return;
	case SchedulerManager::DELAYED:
		logger->Error("EXC start DELAYED");
		return;
	default:
		assert(schedResult == SchedulerManager::DONE);
	}
	logger->Debug("====================[ SCHEDULE ENDED ]====================");
	logger->Debug("Schedule Time: %11.3f[us]", optimization_tmr.getElapsedTimeUs());
	am.PrintStatusReport();
	ra.PrintStatusReport();

	// Check if there is at least one application to synchronize
	if (!am.HasApplications(Application::SYNC)) {
		logger->Debug("NO EXC in SYNC state, synchronization not required");
		return;
	}

	//--- Synchroniztion
	logger->Debug("====================[ SYNC START ]====================");
	optimization_tmr.start();
	syncResult = ym.SyncSchedule();
	optimization_tmr.stop();
	logger->Debug("====================[ SYNC ENDED ]====================");
	logger->Debug("Sync Time: %11.3f[us]", optimization_tmr.getElapsedTimeUs());
	am.PrintStatusReport();
	ra.PrintStatusReport();

}

void ResourceManager::EvtExcStart() {

	logger->Info("EXC Enabled");

	// TODO add here a suitable policy to trigger the optimization

	Optimize();
}

void ResourceManager::EvtExcStop() {

	logger->Info("EXC Disabled");

	// TODO add here a suitable policy to trigger the optimization
	
	// FIXME right now we wait a small timeframe before to trigger a
	// reschedule, in order to avoid a run while an Application is removing a
	// certamin amount of EXC
	::usleep(500000);

	Optimize();
}

void ResourceManager::EvtBbqExit() {
	AppsUidMapIt apps_it;
	AppPtr_t papp;

	logger->Info("Terminating Barbeque...");
	done = true;

	// Stop applications
	papp = am.GetFirst(apps_it);
	for ( ; papp; papp = am.GetNext(apps_it)) {
		// Terminating the application
		logger->Warn("TODO: Send application STOP command");
		// Removing internal data structures
		am.DestroyEXC(papp);
	}

}

void ResourceManager::ControlLoop() {
	std::unique_lock<std::mutex> pendingEvts_ul(pendingEvts_mtx);

	// Wait for a new event
	if (!pendingEvts.any())
		pendingEvts_cv.wait(pendingEvts_ul);

	// Checking for pending events, starting from higer priority ones.
	for(uint8_t evt=EVENTS_COUNT; evt; --evt) {

		logger->Debug("Checking events [%d:%s]",
				evt-1, pendingEvts[evt-1] ? "Pending" : "None");

		// Jump not pending events
		if (!pendingEvts[evt-1])
			continue;

		// Dispatching events to handlers
		switch(evt-1) {
		case EXC_START:
			logger->Debug("Event [EXC_START]");
			EvtExcStart();
			break;
		case EXC_STOP:
			logger->Debug("Event [EXC_STOP]");
			EvtExcStop();
			break;
		case BBQ_EXIT:
			logger->Debug("Event [BBQ_EXIT]");
			EvtBbqExit();
			return;
		case BBQ_ABORT:
			logger->Debug("Event [BBQ_ABORT]");
			logger->Fatal("Abortive quit");
			exit(EXIT_FAILURE);
		default:
			logger->Crit("Unhandled event [%d]", evt-1);
		}

		// Resetting event
		pendingEvts.reset(evt-1);

	}

}

void ResourceManager::Go() {

	Setup();

	while (!done) {
		ControlLoop();
	}

}

} // namespace bbque

