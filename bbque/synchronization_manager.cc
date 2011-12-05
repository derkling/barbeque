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

#include "bbque/synchronization_manager.h"

#include "bbque/application_manager.h"
#include "bbque/configuration_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/system_view.h"
#include "bbque/res/resource_accounter.h"

#include "bbque/app/application.h"
#include "bbque/app/working_mode.h"

#include "bbque/utils/utility.h"

/** Metrics (class COUNTER) declaration */
#define SM_COUNTER_METRIC(NAME, DESC)\
 {SYNCHRONIZATION_MANAGER_NAMESPACE"."NAME, DESC, MetricsCollector::COUNTER, 0}
/** Increase counter for the specified metric */
#define SM_COUNT_EVENT(METRICS, INDEX) \
	mc.Count(METRICS[INDEX].mh);
/** Increase counter for the specified metric */
#define SM_COUNT_EVENT2(METRICS, INDEX, AMOUNT) \
	mc.Count(METRICS[INDEX].mh, AMOUNT);

/** Metrics (class SAMPLE) declaration */
#define SM_SAMPLE_METRIC(NAME, DESC)\
 {SYNCHRONIZATION_MANAGER_NAMESPACE"."NAME, DESC, MetricsCollector::SAMPLE, 0}
/** Reset the timer used to evaluate metrics */
#define SM_RESET_TIMING(TIMER) \
	TIMER.start();
/** Acquire a new completion time sample */
#define SM_GET_TIMING(METRICS, INDEX, TIMER) \
	mc.AddSample(METRICS[INDEX].mh, TIMER.getElapsedTimeMs());
/** Acquire a new EXC reconfigured sample */
#define SM_ADD_SAMPLE(METRICS, INDEX, COUNT) \
	mc.AddSample(METRICS[INDEX].mh, COUNT);

namespace bu = bbque::utils;
namespace bp = bbque::plugins;
namespace po = boost::program_options;
namespace br = bbque::res;

namespace bbque {

/* Definition of metrics used by this module */
MetricsCollector::MetricsCollection_t
SynchronizationManager::metrics[SM_METRICS_COUNT] = {
	//----- Event counting metrics
	SM_COUNTER_METRIC("runs", "SyncP executions count"),
	SM_COUNTER_METRIC("comp", "SyncP completion count"),
	SM_COUNTER_METRIC("excs", "Total EXC reconf count"),
	SM_COUNTER_METRIC("sync_hit",  "Syncs HIT count"),
	SM_COUNTER_METRIC("sync_miss", "Syncs MISS count"),
	//----- Timing metrics
	SM_SAMPLE_METRIC("syncp.avg.time",  "Avg SyncP execution t[ms]"),
	SM_SAMPLE_METRIC("syncp.avg.pre",   "  PreChange  exe t[ms]"),
	SM_SAMPLE_METRIC("syncp.avg.lat",   "  Pre-Sync Lat   t[ms]"),
	SM_SAMPLE_METRIC("syncp.avg.sync",  "  SyncChange exe t[ms]"),
	SM_SAMPLE_METRIC("syncp.avg.synp",  "  SyncPlatform exe t[ms]"),
	SM_SAMPLE_METRIC("syncp.avg.do",    "  DoChange   exe t[ms]"),
	SM_SAMPLE_METRIC("syncp.avg.post",  "  PostChange exe t[ms]"),
	//----- Couting statistics
	SM_SAMPLE_METRIC("avge", "Average EXCs reconf"),
	SM_SAMPLE_METRIC("app.SyncLat", "Average SyncLatency declared"),

};

SynchronizationManager & SynchronizationManager::GetInstance() {
	static SynchronizationManager ym;
	return ym;
}

SynchronizationManager::SynchronizationManager() :
	am(ApplicationManager::GetInstance()),
	ap(ApplicationProxy::GetInstance()),
	mc(bu::MetricsCollector::GetInstance()),
	ra(br::ResourceAccounter::GetInstance()),
	pp(PlatformProxy::GetInstance()),
	sv(SystemView::GetInstance()),
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

	//---------- Setup all the module metrics
	mc.Register(metrics, SM_METRICS_COUNT);

}

SynchronizationManager::~SynchronizationManager() {
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_PreChange(ApplicationStatusIF::SyncState_t syncState) {
	AppsUidMapIt apps_it;

	typedef std::map<AppPtr_t, ApplicationProxy::pPreChangeRsp_t> RspMap_t;
	typedef std::pair<AppPtr_t, ApplicationProxy::pPreChangeRsp_t> RspMapEntry_t;

	SynchronizationPolicyIF::ExitCode_t syncp_result;
	ApplicationProxy::pPreChangeRsp_t presp;
	RspMap_t::iterator resp_it;
	RTLIB_ExitCode_t result;
	RspMap_t rsp_map;
	AppPtr_t papp;

	logger->Debug("STEP 1: preChange() START");
	SM_RESET_TIMING(sm_tmr);

	papp = am.GetFirst(syncState, apps_it);
	for ( ; papp; papp = am.GetNext(syncState, apps_it)) {

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

		// Collect stats on declared sync latency
		SM_ADD_SAMPLE(metrics, SM_SYNCP_APP_SYNCLAT, presp->syncLatency);

		syncp_result = policy->CheckLatency(papp, presp->syncLatency);
		// TODO: check the POLICY required action

		// Remove the respose future
		rsp_map.erase(resp_it);
	}

	// Collecing execution metrics
	SM_GET_TIMING(metrics, SM_SYNCP_TIME_PRECHANGE, sm_tmr);
	logger->Debug("STEP 1: preChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_SyncChange(
		ApplicationStatusIF::SyncState_t syncState) {
	AppsUidMapIt apps_it;

	typedef std::map<AppPtr_t, ApplicationProxy::pSyncChangeRsp_t> RspMap_t;
	typedef std::pair<AppPtr_t, ApplicationProxy::pSyncChangeRsp_t> RspMapEntry_t;

	ApplicationProxy::pSyncChangeRsp_t presp;
	RspMap_t::iterator resp_it;
	RTLIB_ExitCode_t result;
	RspMap_t rsp_map;
	AppPtr_t papp;

	logger->Debug("STEP 2: syncChange() START");
	SM_RESET_TIMING(sm_tmr);

	papp = am.GetFirst(syncState, apps_it);
	for ( ; papp; papp = am.GetNext(syncState, apps_it)) {

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

			// Accounting for syncpoints missed
			SM_COUNT_EVENT(metrics, SM_SYNCP_SYNC_MISS);
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

		// Accounting for syncpoints missed
		SM_COUNT_EVENT(metrics, SM_SYNCP_SYNC_HIT);

		logger->Info("STEP 2: <--------- OK -- [%s]", papp->StrId());

		// Remove the respose future
		rsp_map.erase(resp_it);
	}

	// Collecing execution metrics
	SM_GET_TIMING(metrics, SM_SYNCP_TIME_SYNCCHANGE, sm_tmr);
	logger->Debug("STEP 2: syncChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_DoChange(ApplicationStatusIF::SyncState_t syncState) {
	AppsUidMapIt apps_it;

	RTLIB_ExitCode_t result;
	AppPtr_t papp;

	logger->Debug("STEP 3: doChange() START");
	SM_RESET_TIMING(sm_tmr);

	papp = am.GetFirst(syncState, apps_it);
	for ( ; papp; papp = am.GetNext(syncState, apps_it)) {

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

	// Collecing execution metrics
	SM_GET_TIMING(metrics, SM_SYNCP_TIME_DOCHANGE, sm_tmr);
	logger->Debug("STEP 3: doChange() DONE");

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_PostChange(ApplicationStatusIF::SyncState_t syncState) {
	ApplicationProxy::pPostChangeRsp_t presp;
	RTLIB_ExitCode_t result;
	AppsUidMapIt apps_it;
	AppPtr_t papp;
	uint8_t excs = 0;

	logger->Debug("STEP 4: postChange() START");
	SM_RESET_TIMING(sm_tmr);

	papp = am.GetFirst(syncState, apps_it);
	for ( ; papp; papp = am.GetNext(syncState, apps_it)) {

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

		// Disregarding commit for EXC disabled meanwhile
		if (papp->Disabled())
			continue;

		// Perform resource acquisition for RUNNING App/ExC
		DoAcquireResources(papp);
		excs++;
	}

	// Collecing execution metrics
	SM_GET_TIMING(metrics, SM_SYNCP_TIME_POSTCHANGE, sm_tmr);
	logger->Debug("STEP 4: postChange() DONE");

	// Account for total reconfigured EXCs
	SM_COUNT_EVENT2(metrics, SM_SYNCP_EXCS, excs);

	// Collect statistics on average EXCSs reconfigured.
	SM_ADD_SAMPLE(metrics, SM_SYNCP_AVGE, excs);

	return OK;
}

void SynchronizationManager::DoAcquireResources(AppPtr_t papp) {
	ApplicationManager &am(ApplicationManager::GetInstance());
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	br::ResourceAccounter::ExitCode_t raResult;

	// Acquiring the resources for RUNNING Applications
	if (!papp->Blocking()) {

		logger->Debug("SyncAcquire: [%s] is in %s/%s", papp->StrId(),
				papp->StateStr(papp->State()),
				papp->SyncStateStr(papp->SyncState()));

		// Resource acquisition
		raResult = ra.SyncAcquireResources(papp);

		// If failed abort the single App/ExC sync
		if (raResult != br::ResourceAccounter::RA_SUCCESS) {
			logger->Error("SyncAcquire: failed for [%s]. Returned %d",
					papp->StrId(), raResult);
			am.SyncAbort(papp);
		}
	}

	// Committing change to the ApplicationManager
	// NOTE: this should remove the current app from the queue,
	// otherwise we enter an endless loop
	am.SyncCommit(papp);
}


SynchronizationManager::ExitCode_t
SynchronizationManager::Sync_Platform(ApplicationStatusIF::SyncState_t syncState) {
	PlatformProxy::ExitCode_t result = PlatformProxy::OK;
	AppsUidMapIt apps_it;
	AppPtr_t papp;

	logger->Debug("STEP M: SyncPlatform() START");
	SM_RESET_TIMING(sm_tmr);

	papp = am.GetFirst(syncState, apps_it);
	for ( ; papp; papp = am.GetNext(syncState, apps_it)) {

		logger->Info("STEP M: SyncPlatform() ===> [%s]", papp->StrId());

		// Jumping meanwhile disabled applications
		if (papp->Disabled()) {
			logger->Debug("STEP M: release resources of disabled EXC [%s]",
					papp->StrId());
			pp.ReclaimResources(papp);
		}

		// TODO: reconfigure resources
		switch (syncState) {
		case ApplicationStatusIF::STARTING:
			result = pp.MapResources(papp,
					papp->NextAWM()->GetResourceBinding());
			break;
		case ApplicationStatusIF::RECONF:
		case ApplicationStatusIF::MIGREC:
		case ApplicationStatusIF::MIGRATE:
			result = pp.MapResources(papp,
					papp->NextAWM()->GetResourceBinding());
			break;
		case ApplicationStatusIF::BLOCKED:
			result = pp.ReclaimResources(papp);
			break;
		default:
			break;
		}

		logger->Info("STEP M: <--------- OK -- [%s]", papp->StrId());
	}

	// Collecting execution metrics
	SM_GET_TIMING(metrics, SM_SYNCP_TIME_SYNCPLAT, sm_tmr);
	logger->Debug("STEP M: SyncPlatform() DONE");

	if (result == PlatformProxy::OK)
		return OK;

	return PLATFORM_SYNC_FAILED;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::SyncApps(ApplicationStatusIF::SyncState_t syncState) {
	SynchronizationPolicyIF::SyncLatency_t syncLatency;
	ExitCode_t result;

	if (syncState == ApplicationStatusIF::SYNC_NONE) {
		logger->Warn("Synchronization FAILED (Error: empty EXCs list)");
		assert(syncState != ApplicationStatusIF::SYNC_NONE);
		return OK;
	}

	result = Sync_PreChange(syncState);
	if (result != OK)
		return result;

	// Wait for the policy specified sync point
	syncLatency = policy->EstimatedSyncTime();
	logger->Debug("Wait sync point for %d[ms]", syncLatency);
	std::this_thread::sleep_for(
			std::chrono::milliseconds(syncLatency));
	SM_ADD_SAMPLE(metrics, SM_SYNCP_TIME_LATENCY, syncLatency);

	result = Sync_SyncChange(syncState);
	if (result != OK)
		return result;

	result = Sync_Platform(syncState);
	if (result != OK)
		return result;

	result = Sync_DoChange(syncState);
	if (result != OK)
		return result;

	result = Sync_PostChange(syncState);
	if (result != OK)
		return result;

	return OK;
}

SynchronizationManager::ExitCode_t
SynchronizationManager::SyncSchedule() {
	ApplicationStatusIF::SyncState_t syncState;
	br::ResourceAccounter::ExitCode_t raResult;
	bu::Timer syncp_tmr;
	ExitCode_t result;

	// TODO add here proper tracing/monitoring events for statistics
	// collection

	logger->Info("Synchronization [%d] START", ++sync_count);
	am.ReportStatusQ();
	am.ReportSyncQ();

	// Account for SyncP runs
	SM_COUNT_EVENT(metrics, SM_SYNCP_RUNS);

	// Reset the SyncP overall timer
	SM_RESET_TIMING(syncp_tmr);

	// TODO here a synchronization decision policy is used
	// to decide if a synchronization should be run or not, e.g. based on
	// the kind of applications in SYNC state or considering stability
	// problems and synchronization overheads.
	// The role of the SynchronizationManager it quite simple: it calls a
	// policy provided method which should decide what applications must be
	// synched. As soon as the queue of apps to sync returned is empty, the
	// syncronization is considered terminated and will start again only at
	// the next synchronization event.
	syncState = policy->GetApplicationsQueue(sv, true);

	if (syncState == ApplicationStatusIF::SYNC_NONE) {
		logger->Info("Synchronization [%d] ABORTED", sync_count);
		// Possibly this should never happens
		assert(syncState != ApplicationStatusIF::SYNC_NONE);
		return OK;
	}

	// Start the resource accounter synchronized session
	raResult = ra.SyncStart();
	if (raResult != ResourceAccounter::RA_SUCCESS) {
		logger->Fatal("SynchSchedule: Unable to start resource accounting "
				"sync session");
		return ABORTED;
	}

	while (syncState != ApplicationStatusIF::SYNC_NONE) {

		// Synchronize these policy selected apps
		result = SyncApps(syncState);
		if (result != OK) {
			ra.SyncAbort();
			return result;
		}

		// Select next set of apps to synchronize (if any)
		syncState = policy->GetApplicationsQueue(sv);
	}

	// Commit the resource accounter synchronized session
	raResult = ra.SyncCommit();
	if (raResult != ResourceAccounter::RA_SUCCESS) {
		logger->Fatal("SynchSchedule: Resource accounting sync session commit"
				"failed");
		return ABORTED;
	}

	// Collecing overall SyncP execution time
	SM_GET_TIMING(metrics, SM_SYNCP_TIME, syncp_tmr);

	// Account for SyncP completed
	SM_COUNT_EVENT(metrics, SM_SYNCP_COMP);

	logger->Info("Synchronization [%d] DONE", sync_count);
	am.ReportStatusQ();
	am.ReportSyncQ();

	return OK;
}

} // namespace bbque

