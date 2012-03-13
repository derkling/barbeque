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

#include "bbque/resource_manager.h"

#include <chrono>

#include "bbque/configuration_manager.h"
#include "bbque/plugin_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/signals_manager.h"
#include "bbque/application_manager.h"

#include "bbque/utils/utility.h"

#define RESOURCE_MANAGER_NAMESPACE "bq.rm"

/** Metrics (class COUNTER) declaration */
#define RM_COUNTER_METRIC(NAME, DESC)\
 {RESOURCE_MANAGER_NAMESPACE"."NAME, DESC, MetricsCollector::COUNTER, 0}
/** Increase counter for the specified metric */
#define RM_COUNT_EVENT(METRICS, INDEX) \
	mc.Count(METRICS[INDEX].mh);
/** Increase counter for the specified metric */
#define RM_COUNT_EVENTS(METRICS, INDEX, AMOUNT) \
	mc.Count(METRICS[INDEX].mh, AMOUNT);

/** Metrics (class SAMPLE) declaration */
#define RM_SAMPLE_METRIC(NAME, DESC)\
 {RESOURCE_MANAGER_NAMESPACE"."NAME, DESC, MetricsCollector::SAMPLE, 0}
/** Reset the timer used to evaluate metrics */
#define RM_RESET_TIMING(TIMER) \
	TIMER.start();
/** Acquire a new time sample */
#define RM_GET_TIMING(METRICS, INDEX, TIMER) \
	mc.AddSample(METRICS[INDEX].mh, TIMER.getElapsedTimeMs());
/** Acquire a new (generic) sample */
#define RM_ADD_SAMPLE(METRICS, INDEX, SAMPLE) \
	mc.AddSample(METRICS[INDEX].mh, SAMPLE);

/** Metrics (class PERIDO) declaration */
#define RM_PERIOD_METRIC(NAME, DESC)\
 {RESOURCE_MANAGER_NAMESPACE"."NAME, DESC, MetricsCollector::PERIOD, 0}
/** Acquire a new time sample */
#define RM_GET_PERIOD(METRICS, INDEX, PERIOD) \
	mc.PeriodSample(METRICS[INDEX].mh, PERIOD);

#define LNSCHB "::::::::::::::::::::: SCHEDULE START ::::::::::::::::::::::::"
#define LNSCHE ":::::::::::::::::::::  SCHEDULE END  ::::::::::::::::::::::::"
#define LNSYNB "**********************  SYNC START  *************************"
#define LNSYNF "*********************  SYNC FAILED  *************************"
#define LNSYNE "***********************  SYNC END  **************************"

namespace br = bbque::res;
namespace bu = bbque::utils;
namespace bp = bbque::plugins;
namespace po = boost::program_options;

using std::chrono::milliseconds;

namespace bbque {

/* Definition of metrics used by this module */
MetricsCollector::MetricsCollection_t
ResourceManager::metrics[RM_METRICS_COUNT] = {

	//----- Event counting metrics
	RM_COUNTER_METRIC("evt.tot",	"Total events"),
	RM_COUNTER_METRIC("evt.start",	"  START events"),
	RM_COUNTER_METRIC("evt.stop",	"  STOP  events"),
	RM_COUNTER_METRIC("evt.opts",	"  OPTS  events"),
	RM_COUNTER_METRIC("evt.usr1",	"  USR1  events"),
	RM_COUNTER_METRIC("evt.usr2",	"  USR2  events"),

	RM_COUNTER_METRIC("sch.tot",	"Total Scheduler activations"),
	RM_COUNTER_METRIC("sch.failed",	"  FAILED  schedules"),
	RM_COUNTER_METRIC("sch.delayed","  DELAYED schedules"),
	RM_COUNTER_METRIC("sch.empty",	"  EMPTY   schedules"),

	RM_COUNTER_METRIC("syn.tot",	"Total Synchronization activations"),
	RM_COUNTER_METRIC("syn.failed",	"  FAILED synchronizations"),

	//----- Sampling statistics
	RM_SAMPLE_METRIC("evt.avg.time",  "Avg events processing t[ms]"),
	RM_SAMPLE_METRIC("evt.avg.start", "  START events"),
	RM_SAMPLE_METRIC("evt.avg.stop",  "  STOP  events"),
	RM_SAMPLE_METRIC("evt.avg.opts",  "  OPTS  events"),
	RM_SAMPLE_METRIC("evt.avg.usr1",  "  USR1  events"),
	RM_SAMPLE_METRIC("evt.avg.usr2",  "  USR2  events"),

	RM_PERIOD_METRIC("evt.per",		"Avg events period t[ms]"),
	RM_PERIOD_METRIC("evt.per.start",	"  START events"),
	RM_PERIOD_METRIC("evt.per.stop",	"  STOP  events"),
	RM_PERIOD_METRIC("evt.per.opts",	"  OPTS  events"),
	RM_PERIOD_METRIC("evt.per.usr1",	"  USR1  events"),
	RM_PERIOD_METRIC("evt.per.usr2",	"  USR2  events"),

	RM_PERIOD_METRIC("sch.per",   "Avg Scheduler period t[ms]"),
	RM_PERIOD_METRIC("syn.per",   "Avg Synchronization period t[ms]"),

};


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
	ra(ResourceAccounter::GetInstance()),
	mc(bu::MetricsCollector::GetInstance()),
	pp(PlatformProxy::GetInstance()),
	opt("rm.opt", std::bind(&ResourceManager::Optimize, this)) {

	//---------- Setup all the module metrics
	mc.Register(metrics, RM_METRICS_COUNT);
}

ResourceManager::~ResourceManager() {
}

ResourceManager::ExitCode_t
ResourceManager::Setup() {

	//---------- Get a logger module
	std::string logger_name(RESOURCE_MANAGER_NAMESPACE);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "RM: Logger module creation FAILED\n");
		assert(logger);
		return SETUP_FAILED;
	}

	//---------- Dump list of registered plugins
	const bp::PluginManager::RegistrationMap & rm = pm.GetRegistrationMap();
	logger->Info("RM: Registered plugins:");
	bp::PluginManager::RegistrationMap::const_iterator i;
	for (i = rm.begin(); i != rm.end(); ++i)
		logger->Info(" * %s", (*i).first.c_str());

	//---------- Init Platform Integration Layer (PIL)
	PlatformProxy::ExitCode_t result = pp.LoadPlatformData();
	if (result != PlatformProxy::OK) {
		logger->Fatal("Platform Integration Layer initialization FAILED!");
		return SETUP_FAILED;
	}

	//---------- Start bbque services
	ap.Start();
	pp.Start();

	return OK;
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
	static bu::Timer optimization_tmr;
	double period;

	// Check if there is at least one application to synchronize
	if ((!am.HasApplications(Application::READY)) &&
			(!am.HasApplications(Application::RUNNING))) {
		logger->Debug("NO active EXCs, re-scheduling not required");
		return;
	}

	ra.PrintStatusReport();
	am.PrintStatusReport();
	logger->Info("Running Optimization...");

	// Account for a new schedule activation
	RM_COUNT_EVENT(metrics, RM_SCHED_TOTAL);
	RM_GET_PERIOD(metrics, RM_SCHED_PERIOD, period);

	//--- Scheduling
	logger->Notice(LNSCHB);
	optimization_tmr.start();
	schedResult = sm.Schedule();
	optimization_tmr.stop();
	switch(schedResult) {
	case SchedulerManager::MISSING_POLICY:
	case SchedulerManager::FAILED:
		logger->Warn("Schedule FAILED (Error: scheduling policy failed)");
		RM_COUNT_EVENT(metrics, RM_SCHED_FAILED);
		return;
	case SchedulerManager::DELAYED:
		logger->Error("Schedule DELAYED");
		RM_COUNT_EVENT(metrics, RM_SCHED_DELAYED);
		return;
	default:
		assert(schedResult == SchedulerManager::DONE);
	}
	logger->Info(LNSCHE);
	logger->Notice("Schedule Time: %11.3f[us]", optimization_tmr.getElapsedTimeUs());
	ra.PrintStatusReport(true);
	am.PrintStatusReport(true);

	// Check if there is at least one application to synchronize
	if (!am.HasApplications(Application::SYNC)) {
		logger->Debug("NO EXC in SYNC state, synchronization not required");
		RM_COUNT_EVENT(metrics, RM_SCHED_EMPTY);
		return;
	}

	// Account for a new synchronizaiton activation
	RM_COUNT_EVENT(metrics, RM_SYNCH_TOTAL);
	RM_GET_PERIOD(metrics, RM_SYNCH_PERIOD, period);
	if (period)
		logger->Notice("Schedule Run-time: %9.3f[ms]", period);

	//--- Synchroniztion
	logger->Notice(LNSYNB);
	optimization_tmr.start();
	syncResult = ym.SyncSchedule();
	optimization_tmr.stop();
	if (syncResult != SynchronizationManager::OK) {
		logger->Warn(LNSYNF);
		RM_COUNT_EVENT(metrics, RM_SYNCH_FAILED);
		// FIXME here we should implement some counter-meaure to
		// ensure consistency
		return;
	}
	logger->Info(LNSYNE);
	ra.PrintStatusReport(0, true);
	am.PrintStatusReport(true);
	logger->Notice("Sync Time: %11.3f[us]", optimization_tmr.getElapsedTimeUs());

}

void ResourceManager::EvtExcStart() {

	logger->Info("EXC Enabled");

	// Reset timer for START event execution time collection
	RM_RESET_TIMING(rm_tmr);

	// TODO add here a suitable policy to trigger the optimization

	opt.Schedule();
	
	// Collecing execution metrics
	RM_GET_TIMING(metrics, RM_EVT_TIME_START, rm_tmr);
}

void ResourceManager::EvtExcStop() {

	logger->Info("EXC Disabled");

	// Reset timer for START event execution time collection
	RM_RESET_TIMING(rm_tmr);

	// TODO add here a suitable policy to trigger the optimization
	
	// FIXME right now we wait a small timeframe before to trigger a
	// reschedule, in order to avoid a run while an Application is removing a
	// certamin amount of EXC
	opt.Schedule(milliseconds(500));

	// Collecing execution metrics
	RM_GET_TIMING(metrics, RM_EVT_TIME_STOP, rm_tmr);
}

void ResourceManager::EvtBbqOpts() {

	logger->Info("BBQ Optimization Request");

	// Reset timer for START event execution time collection
	RM_RESET_TIMING(rm_tmr);

	// TODO add here a suitable policy to trigger the optimization

	opt.Schedule();

	// Collecing execution metrics
	RM_GET_TIMING(metrics, RM_EVT_TIME_OPTS, rm_tmr);
}


void ResourceManager::EvtBbqUsr1() {

	// Reset timer for START event execution time collection
	RM_RESET_TIMING(rm_tmr);

	logger->Info("");
	logger->Info("==========[ Status Queues ]============"
			"========================================");
	logger->Info("");
	am.ReportStatusQ(true);

	logger->Info("");
	logger->Info("");
	logger->Info("==========[ Synchronization Queues ]==="
			"========================================");
	logger->Info("");
	am.ReportSyncQ(true);

	logger->Notice("");
	logger->Notice("");
	logger->Notice("==========[ Resources Status ]========="
			"========================================");
	logger->Notice("");
	ra.PrintStatusReport(0, true);

	logger->Notice("");
	logger->Notice("");
	logger->Notice("==========[ EXCs Status ]=============="
			"========================================");
	logger->Notice("");
	am.PrintStatusReport(true);

	// Clear the corresponding event flag
	logger->Notice("");
	pendingEvts.reset(BBQ_USR1);

	// Collecing execution metrics
	RM_GET_TIMING(metrics, RM_EVT_TIME_USR1, rm_tmr);
}

void ResourceManager::EvtBbqUsr2() {

	// Reset timer for START event execution time collection
	RM_RESET_TIMING(rm_tmr);

	logger->Debug("Dumping metrics collection...");
	mc.DumpMetrics();

	// Clear the corresponding event flag
	pendingEvts.reset(BBQ_USR2);

	// Collecing execution metrics
	RM_GET_TIMING(metrics, RM_EVT_TIME_USR2, rm_tmr);
}

void ResourceManager::EvtBbqExit() {
	AppsUidMapIt apps_it;
	AppPtr_t papp;

	logger->Notice("Terminating Barbeque...");
	done = true;

	// Dumping collected stats before termination
	EvtBbqUsr1();
	EvtBbqUsr2();

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
	double period;

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

		// Account for a new event
		RM_COUNT_EVENT(metrics, RM_EVT_TOTAL);
		RM_GET_PERIOD(metrics, RM_EVT_PERIOD, period);

		// Dispatching events to handlers
		switch(evt-1) {
		case EXC_START:
			logger->Debug("Event [EXC_START]");
			EvtExcStart();
			RM_COUNT_EVENT(metrics, RM_EVT_START);
			RM_GET_PERIOD(metrics, RM_EVT_PERIOD_START, period);
			break;
		case EXC_STOP:
			logger->Debug("Event [EXC_STOP]");
			EvtExcStop();
			RM_COUNT_EVENT(metrics, RM_EVT_STOP);
			RM_GET_PERIOD(metrics, RM_EVT_PERIOD_STOP, period);
			break;
		case BBQ_OPTS:
			logger->Debug("Event [BBQ_OPTS]");
			EvtBbqOpts();
			RM_COUNT_EVENT(metrics, RM_EVT_OPTS);
			RM_GET_PERIOD(metrics, RM_EVT_PERIOD_OPTS, period);
			break;
		case BBQ_USR1:
			logger->Debug("Event [BBQ_USR1]");
			RM_COUNT_EVENT(metrics, RM_EVT_USR1);
			RM_GET_PERIOD(metrics, RM_EVT_PERIOD_USR1, period);
			EvtBbqUsr1();
			return;
		case BBQ_USR2:
			logger->Debug("Event [BBQ_USR2]");
			RM_COUNT_EVENT(metrics, RM_EVT_USR2);
			RM_GET_PERIOD(metrics, RM_EVT_PERIOD_USR2, period);
			EvtBbqUsr2();
			return;
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

ResourceManager::ExitCode_t
ResourceManager::Go() {
	ExitCode_t result;

	result = Setup();
	if (result != OK)
		return result;

	while (!done) {
		ControlLoop();
	}

	return OK;
}

} // namespace bbque

