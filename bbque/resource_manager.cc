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
#include "bbque/res/resource_accounter.h"

#include "bbque/utils/utility.h"

namespace br = bbque::res;
namespace bp = bbque::plugins;
namespace po = boost::program_options;

namespace bbque {

ResourceManager & ResourceManager::GetInstance() {
	static ResourceManager rtrm;

	return rtrm;
}

ResourceManager::ResourceManager() :
	ps(PlatformServices::GetInstance()),
	pm(plugins::PluginManager::GetInstance()),
	rs(ResourceScheduler::GetInstance()),
	sm(SynchronizationManager::GetInstance()),
	am(ApplicationManager::GetInstance()),
	ap(ApplicationProxy::GetInstance()) {

}

ResourceManager::~ResourceManager() {
}

void ResourceManager::Setup() {

	//---------- Get a logger module
	std::string logger_name("bq.rm");
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
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

void ResourceManager::EvtExcStart() {
	ApplicationManager &am(ApplicationManager::GetInstance());
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	SynchronizationManager::ExitCode_t syncResult;
	ResourceScheduler::ExitCode_t schedResult;

	logger->Info("EXC Enabled: Running Optimization...");

	am.PrintStatusReport();
	ra.PrintStatusReport();

	logger->Debug(">>>>> SCHEDULE START");
	schedResult = rs.Schedule();
	switch(schedResult) {
	case ResourceScheduler::MISSING_POLICY:
	case ResourceScheduler::FAILED:
		logger->Error("EXC start FAILED (Error: scheduling policy failed)");
		return;
	case ResourceScheduler::DELAYED:
		logger->Error("EXC start DELAYED");
		return;
	default:
		assert(schedResult == ResourceScheduler::DONE);
	}
	logger->Debug("<<<<< SCHEDULE ENDED");
	
	am.PrintStatusReport();
	ra.PrintStatusReport();

	logger->Debug(">>>>> SYNC START");
	syncResult = sm.SyncSchedule();
	logger->Debug("<<<<< SYNC ENDED");

	am.PrintStatusReport();
	ra.PrintStatusReport();

}

void ResourceManager::EvtBbqExit() {
	ApplicationManager & am = ApplicationManager::GetInstance();
	//ApplicationProxy & ap = ApplicationProxy::GetInstance();
	AppsUidMap_t const * apps = am.Applications();
	AppsUidMap_t::const_iterator it = apps->begin();
	AppPtr_t papp;

	logger->Info("Terminating Barbeque...");
	done = true;

	// Stop applications
	for ( ; it != apps->end(); ++it) {
		papp = (*it).second;
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
			logger->Debug("Event [EXC_STOP");
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

