/**
 *       @file  synchronization_manager.cc
 *      @brief  The module to synchronize applicaitons status
 *
 * This module provides a unified interface to access application status
 * synchronization primitives. Once a new resource scheduling has been
 * computed, the status of registered application should be updated according
 * to the new schedule. This update requires to communicate each Execution
 * Context its new assigned set of resources and to verify that the actual
 * resources usage by each application match the schedule. Some of these operations
 * are delagated to module plugins, while the core glue code for status
 * synchronization is defined by this class
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  06/03/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "bbque/synchronization_manager.h"

#include "bbque/configuration_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/system_view.h"

#include "bbque/app/application.h"

#include "bbque/utils/utility.h"

#define SYNCHRONIZATION_MANAGER_NAMESPACE "bq.sm"

namespace bp = bbque::plugins;
namespace po = boost::program_options;

namespace bbque {

SynchronizationManager & SynchronizationManager::GetInstance() {
	static SynchronizationManager sm;
	return sm;
}

SynchronizationManager::SynchronizationManager() {
	std::string sync_policy;

	//---------- Get a logger module
	bp::LoggerIF::Configuration conf(SYNCHRONIZATION_MANAGER_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "SM: Logger module creation FAILED\n");
		assert(logger);
	}

	logger->Debug("Starting synchronization manager...");

	//---------- Loading module configuration
	ConfigurationManager & cm = ConfigurationManager::GetInstance();
	po::options_description opts_desc("Synchronization Manager Options");
	opts_desc.add_options()
		("SynchronizationManager.policy",
		 po::value<std::string>
		 (&sync_policy)->default_value(
			 BBQUE_DEFAULT_SYNCHRONIZATION_MANAGER_POLICY),
		 "The name of the optimization policy to use")
		;
	po::variables_map opts_vm;
	cm.ParseConfigurationFile(opts_desc, opts_vm);

	//---------- Load the required optimization plugin
	std::string sync_namespace(SYNCHRONIZATION_POLICY_NAMESPACE);
	logger->Debug("Loading synchronization policy [%s%s]...",
			sync_namespace.c_str(), sync_policy.c_str());
	policy = ModulesFactory::GetSynchronizationPolicyModule(
			sync_namespace + sync_policy);
	if (!policy) {
		logger->Fatal("Synchronization policy load FAILED "
			"(Error: missing plugin for [%s%s])",
			sync_namespace.c_str(), sync_policy.c_str());
		assert(policy);
	}

}

SynchronizationManager::~SynchronizationManager() {
}


SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_PreChange(AppsUidMap_t const *apps) {
	AppsMap_t::const_iterator it(apps->begin());
	AppPtr_t papp;

	logger->Debug("STEP 1: preChange() START");

	for ( ; it != apps->end(); ++it) {
		papp = (*it).second;

		if (!policy->DoSync(papp))
			continue;

		logger->Debug("STEP 1: preChange() ===> [%s]", papp->StrId());

	}

	logger->Debug("STEP 1: preChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_SyncChange(AppsUidMap_t const *apps) {
	AppsMap_t::const_iterator it(apps->begin());
	AppPtr_t papp;

	logger->Debug("STEP 2: syncChange() START");

	for ( ; it != apps->end(); ++it) {
		papp = (*it).second;

		if (!policy->DoSync(papp))
			continue;

		logger->Debug("STEP 2: syncChange() ===> [%s]", papp->StrId());

	}

	logger->Debug("STEP 2: syncChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_DoChange(AppsUidMap_t const *apps) {
	AppsMap_t::const_iterator it(apps->begin());
	AppPtr_t papp;

	logger->Debug("STEP 3: doChange() START");

	for ( ; it != apps->end(); ++it) {
		papp = (*it).second;

		if (!policy->DoSync(papp))
			continue;

		logger->Debug("STEP 3: doChange() ===> [%s]", papp->StrId());

	}

	logger->Debug("STEP 3: doChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_PostChange(AppsUidMap_t const *apps) {
	ApplicationManager &am(ApplicationManager::GetInstance());
	AppsMap_t::const_iterator it(apps->begin());
	AppPtr_t papp;

	logger->Debug("STEP 4: postChange() START");

	while (it != apps->end()) {
		papp = (*it).second;

		if (!policy->DoSync(papp))
			continue;

		logger->Debug("STEP 4: postChange() ===> [%s]", papp->StrId());

		// Committing change to the ApplicationManager
		am.SyncCommit(papp);
		
		// Get next app on the queue
		it = apps->begin();
	}
	
	logger->Debug("STEP 4: postChange() DONE");

	return OK;
}
SynchronizationManager::ExitCode_t
SynchronizationManager::SyncApps(AppsUidMap_t const *apps) {
	AppsMap_t::const_iterator it;
	ExitCode_t result;

	if (apps->size() == 0) {
		logger->Warn("Synchronization FAILED (Error: empty EXCs list)");
		assert(apps->size());
		return OK;
	}

	result = Sync_PreChange(apps);
	if (result != OK)
		return result;

	logger->Debug("STEP 2: syncChange()");

	logger->Debug("STEP 3: doChange()");

	result = Sync_PostChange(apps);
	if (result != OK)
		return result;

	return OK;
}



SynchronizationManager::ExitCode_t
SynchronizationManager::SyncSchedule() {
	static SystemView &sv(SystemView::GetInstance());
	AppsUidMap_t const *apps = NULL;
	ExitCode_t result;

	// TODO add here proper tracing/monitoring events for statistics
	// collection

	logger->Info("Synchronization START");

	// TODO here a synchronization decision policy is used
	// to decide if a synchronization should be run or not, e.g. based on
	// the kind of applications in SYNC state or considering stability
	// problems and synchronization overheads.
	// The role of the SynchronizationManager it quite simple: it calls a
	// policy provided method which should decide what applications must be
	// synched. As soon as the queue of apps to sync returned is empty, the
	// syncronization is considered terminated and will start again only at
	// the next synchronization event.
	apps = policy->GetApplicationsQueue(sv, true);

	while (apps) {

		// Synchronize these policy selected apps
		result = SyncApps(apps);
		if (result!=OK)
			return result;

		// Select next set of apps to synchronize (if any)
		apps = policy->GetApplicationsQueue(sv);

	}

	logger->Info("Synchronization DONE");

	return OK;

}

} // namespace bbque

