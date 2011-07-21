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
#include "bbque/res/resource_accounter.h"

#include "bbque/app/application.h"
#include "bbque/app/working_mode.h"

#include "bbque/utils/utility.h"

#define SYNCHRONIZATION_MANAGER_NAMESPACE "bq.ym"

namespace bp = bbque::plugins;
namespace po = boost::program_options;
namespace br = bbque::res;

namespace bbque {

SynchronizationManager & SynchronizationManager::GetInstance() {
	static SynchronizationManager ym;
	return ym;
}

SynchronizationManager::SynchronizationManager() :
	sync_count(0) {
	std::string sync_policy;

	//---------- Get a logger module
	bp::LoggerIF::Configuration conf(SYNCHRONIZATION_MANAGER_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "YM: Logger module creation FAILED\n");
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
	ApplicationProxy &ap(ApplicationProxy::GetInstance());
	AppsUidMap_t::const_iterator apps_it(apps->begin());

	typedef std::map<AppPtr_t, ApplicationProxy::pPreChangeRsp_t> RspMap_t;
	typedef std::pair<AppPtr_t, ApplicationProxy::pPreChangeRsp_t> RspMapEntry_t;

	ApplicationProxy::pPreChangeRsp_t presp;
	RspMap_t::iterator resp_it;
	RTLIB_ExitCode_t result;
	RspMap_t rsp_map;
	AppPtr_t papp;

	logger->Debug("STEP 1: preChange() START");

	for ( ; apps_it != apps->end(); ++apps_it) {
		papp = (*apps_it).second;
		assert(papp);

		if (!policy->DoSync(papp))
			continue;

		logger->Info("STEP 1: preChange() ===> [%s]", papp->StrId());

		// Jumping meanwhile disabled applications
		if (papp->Disabled()) {
			logger->Debug("STEP 1: ignoring disabled EXC [%s]",
					papp->StrId());
			continue;
		}

		// Start an Async Pre-Change
		presp = ApplicationProxy::pPreChangeRsp_t(
				new ApplicationProxy::preChangeRsp_t());
		result = ap.SyncP_PreChange_Async(papp, presp);
		if (result != RTLIB_OK)
			continue;

		// Mapping the response future for responses collection
		rsp_map.insert(RspMapEntry_t(papp, presp));

	}

	// Collecting EXC responses
	for (resp_it = rsp_map.begin();
			resp_it != rsp_map.end();
			++resp_it) {

		papp  = (*resp_it).first;
		presp = (*resp_it).second;

		// Jumping meanwhile disabled applications
		if (papp->Disabled()) {
			logger->Debug("STEP 1: ignoring disabled EXC [%s]",
					papp->StrId());
			// Remove the respose future
			rsp_map.erase(resp_it);
			continue;
		}

		logger->Debug("STEP 1: .... (wait) .... [%s]", papp->StrId());
		result = ap.SyncP_PreChange_GetResult(presp);


		if (result == RTLIB_BBQUE_CHANNEL_TIMEOUT) {
			logger->Warn("STEP 1: <---- TIMEOUT -- [%s]",
					papp->StrId());
			// Disabling not responding applications
			papp->Disable();
			// Remove the respose future
			rsp_map.erase(resp_it);
			continue;
		}

		if (result != RTLIB_OK) {
			logger->Warn("STEP 1: <----- FAILED -- [%s]", papp->StrId());
			// FIXME This case should be handled
			assert(false);
		}

		logger->Info("STEP 1: <--------- OK -- [%s]", papp->StrId());
		logger->Info("STEP 1: [%s] declared syncLatency %d[ms]",
				papp->StrId(), presp->syncLatency);

		// TODO Here the synchronization policy should be queryed to
		// decide if the synchronization latency is compliant with the
		// RTRM optimization goals.
		logger->Warn("TODO: Check sync policy for "
				"(%d[ms]) syncLatency compliance",
				presp->syncLatency);

		// Remove the respose future
		rsp_map.erase(resp_it);
	}

	logger->Debug("STEP 1: preChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_SyncChange(AppsUidMap_t const *apps) {
	ApplicationProxy &ap(ApplicationProxy::GetInstance());
	AppsUidMap_t::const_iterator apps_it(apps->begin());

	typedef std::map<AppPtr_t, ApplicationProxy::pSyncChangeRsp_t> RspMap_t;
	typedef std::pair<AppPtr_t, ApplicationProxy::pSyncChangeRsp_t> RspMapEntry_t;

	ApplicationProxy::pSyncChangeRsp_t presp;
	RspMap_t::iterator resp_it;
	RTLIB_ExitCode_t result;
	RspMap_t rsp_map;
	AppPtr_t papp;

	logger->Debug("STEP 2: syncChange() START");

	for ( ; apps_it != apps->end(); ++apps_it) {
		papp = (*apps_it).second;
		assert(papp);

		if (!policy->DoSync(papp))
			continue;

		logger->Info("STEP 2: syncChange() ===> [%s]", papp->StrId());

		// Jumping meanwhile disabled applications
		if (papp->Disabled()) {
			logger->Debug("STEP 2: ignoring disabled EXC [%s]",
					papp->StrId());
			continue;
		}

		// Start an Async Sync-Change
		presp = ApplicationProxy::pSyncChangeRsp_t(
				new ApplicationProxy::syncChangeRsp_t());
		result = ap.SyncP_SyncChange_Async(papp, presp);
		if (result != RTLIB_OK)
			continue;

		// Mapping the response future for responses collection
		rsp_map.insert(RspMapEntry_t(papp, presp));

	}

	// Collecting EXC responses
	for (resp_it = rsp_map.begin();
			resp_it != rsp_map.end();
			++resp_it) {

		papp  = (*resp_it).first;
		presp = (*resp_it).second;

		// Jumping meanwhile disabled applications
		if (papp->Disabled()) {
			logger->Debug("STEP 2: ignoring disabled EXC [%s]",
					papp->StrId());
			// Remove the respose future
			rsp_map.erase(resp_it);
			continue;
		}

		logger->Debug("STEP 2: .... (wait) .... [%s]", papp->StrId());
		result = ap.SyncP_SyncChange_GetResult(presp);

		if (result == RTLIB_BBQUE_CHANNEL_TIMEOUT) {
			logger->Warn("STEP 2: <---- TIMEOUT -- [%s]",
					papp->StrId());
			// Disabling not responding applications
			papp->Disable();
			// Remove the respose future
			rsp_map.erase(resp_it);
			continue;
		}

		if (result != RTLIB_OK) {
			logger->Warn("STEP 2: <----- FAILED -- [%s]", papp->StrId());
			// TODO Here the synchronization policy should be queryed to
			// decide if the synchronization latency is compliant with the
			// RTRM optimization goals.
			//
			logger->Warn("TODO: Check sync policy for sync miss reaction");

			// FIXME This case should be handled
			assert(false);
		}

		logger->Info("STEP 2: <--------- OK -- [%s]", papp->StrId());

		// Remove the respose future
		rsp_map.erase(resp_it);
	}

	logger->Debug("STEP 2: syncChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_DoChange(AppsUidMap_t const *apps) {
	ApplicationProxy &ap(ApplicationProxy::GetInstance());
	AppsUidMap_t::const_iterator apps_it(apps->begin());
	RTLIB_ExitCode_t result;
	AppPtr_t papp;

	logger->Debug("STEP 3: doChange() START");

	for ( ; apps_it != apps->end(); ++apps_it) {
		papp = (*apps_it).second;
		assert(papp);

		if (!policy->DoSync(papp))
			continue;

		logger->Info("STEP 3: doChange() ===> [%s]", papp->StrId());

		// Jumping meanwhile disabled applications
		if (papp->Disabled()) {
			logger->Debug("STEP 3: ignoring disabled EXC [%s]",
					papp->StrId());
			continue;
		}

		// Send a Do-Change
		result = ap.SyncP_DoChange(papp);
		if (result != RTLIB_OK)
			continue;

		logger->Info("STEP 3: <--------- OK -- [%s]", papp->StrId());

	}

	logger->Debug("STEP 3: doChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_PostChange(AppsUidMap_t const *apps) {
	ApplicationManager &am(ApplicationManager::GetInstance());
	ApplicationProxy &ap(ApplicationProxy::GetInstance());
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	AppsUidMap_t::const_iterator apps_it(apps->begin());
	ApplicationProxy::pPostChangeRsp_t presp;
	RTLIB_ExitCode_t result;
	br::ResourceAccounter::ExitCode_t raResult;
	AppPtr_t papp;

	logger->Debug("STEP 4: postChange() START");

	// FIXME: ensure the current app has been removed from the queue
	// otherwise we enter an endless loop
	// This loop solution is used since it is safer to loop on
	// queues which elemenbts could be modified
	apps_it = apps->begin();
	for ( ; apps_it != apps->end(); apps_it = apps->begin()) {

		papp = (*apps_it).second;
		assert(papp);

		if (!policy->DoSync(papp))
			continue;

		logger->Info("STEP 4: postChange() ===> [%s]", papp->StrId());

		// Jumping meanwhile disabled applications
		if (papp->Disabled()) {
			logger->Debug("STEP 4: ignoring disabled EXC [%s]",
					papp->StrId());
			continue;
		}

		// Send a Post-Change (blocking on apps being reconfigured)
		presp = ApplicationProxy::pPostChangeRsp_t(
				new ApplicationProxy::postChangeRsp_t());
		result = ap.SyncP_PostChange(papp, presp);
		
		if (result == RTLIB_BBQUE_CHANNEL_TIMEOUT) {
			logger->Warn("STEP 4: <---- TIMEOUT -- [%s]",
					papp->StrId());
			// Disabling not responding applications
			papp->Disable();
			continue;
		}

		if (result != RTLIB_OK)
			continue;

		logger->Info("STEP 4: <--------- OK -- [%s]", papp->StrId());

		// TODO Here we should collect reconfiguration statistics
		logger->Warn("TODO: Collect reconf statistics");

		// Acquiring the resources for RUNNING Applications
		if (!papp->Blocking()) {

			logger->Debug("SyncAcquire: %s is in %s/%s", papp->StrId(),
					papp->StateStr(papp->State()),
					papp->SyncStateStr(papp->SyncState()));

			raResult = ra.SyncAcquireResources(papp,
				papp->NextAWM()->GetResourceBinding());

			// TODO: Investigate a response action for error return code.
			// 	Evaluate the implementation of a SyncAbort into the
			// 	ApplicationManage
			assert (raResult == br::ResourceAccounter::RA_SUCCESS);
		}

		// Committing change to the ApplicationManager
		// NOTE: this should remove the current app from the queue,
		// otherwise we enter an endless loop
		am.SyncCommit(papp);
	}

	logger->Debug("STEP 4: postChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::SyncApps(AppsUidMap_t const *apps) {
	AppsUidMap_t::const_iterator it;
	ExitCode_t result;

	if (apps->empty()) {
		logger->Warn("Synchronization FAILED (Error: empty EXCs list)");
		assert(!apps->empty());
		return OK;
	}

	result = Sync_PreChange(apps);
	if (result != OK)
		return result;

	result = Sync_SyncChange(apps);
	if (result != OK)
		return result;

	result = Sync_DoChange(apps);
	if (result != OK)
		return result;

	result = Sync_PostChange(apps);
	if (result != OK)
		return result;

	return OK;
}



SynchronizationManager::ExitCode_t
SynchronizationManager::SyncSchedule() {
	static SystemView &sv(SystemView::GetInstance());
	static ApplicationManager &am(ApplicationManager::GetInstance());
	static ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	AppsUidMap_t const *apps = NULL;
	ExitCode_t result;
	br::ResourceAccounter::ExitCode_t raResult;

	// TODO add here proper tracing/monitoring events for statistics
	// collection

	logger->Info("Synchronization [%d] START", ++sync_count);
	am.ReportStatusQ();
	am.ReportSyncQ();

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

	if (!apps) {
		logger->Info("Synchronization [%d] ABORTED", sync_count);
		// Possibly this should never happens
		assert(apps);
		return OK;
	}

	// Start the resource accounter synchronized session
	raResult = ra.SyncStart();
	if (raResult != ResourceAccounter::RA_SUCCESS) {
		logger->Fatal("SynchSchedule: Unable to start resource accounting "
				"sync session");
		return ABORTED;
	}

	while (apps) {

		// Synchronize these policy selected apps
		result = SyncApps(apps);
		if (result != OK) {
			ra.SyncAbort();
			return result;
		}

		// Select next set of apps to synchronize (if any)
		apps = policy->GetApplicationsQueue(sv);
	}

	// Commit the resource accounter synchronized session
	raResult = ra.SyncCommit();
	if (raResult != ResourceAccounter::RA_SUCCESS) {
		logger->Fatal("SynchSchedule: Resource accounting sync session commit"
				"failed");
		return ABORTED;
	}

	logger->Info("Synchronization [%d] DONE", sync_count);
	am.ReportStatusQ();
	am.ReportSyncQ();

	return OK;

}

} // namespace bbque

