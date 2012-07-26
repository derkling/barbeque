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

#include "bbque/scheduler_manager.h"

#include "bbque/configuration_manager.h"
#include "bbque/plugin_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/system.h"

#include "bbque/utils/utility.h"


/** Metrics (class COUNTER) declaration */
#define SM_COUNTER_METRIC(NAME, DESC)\
 {SCHEDULER_MANAGER_NAMESPACE"."NAME, DESC, \
	 MetricsCollector::COUNTER, 0, NULL, 0}
/** Increase counter for the specified metric */
#define SM_COUNT_EVENT(METRICS, INDEX) \
	mc.Count(METRICS[INDEX].mh);
/** Increase counter for the specified metric */
#define SM_COUNT_EVENTS(METRICS, INDEX, AMOUNT) \
	mc.Count(METRICS[INDEX].mh, AMOUNT);

/** Metrics (class SAMPLE) declaration */
#define SM_SAMPLE_METRIC(NAME, DESC)\
 {SCHEDULER_MANAGER_NAMESPACE"."NAME, DESC, \
	 MetricsCollector::SAMPLE, 0, NULL, 0}
/** Reset the timer used to evaluate metrics */
#define SM_RESET_TIMING(TIMER) \
	TIMER.start();
/** Acquire a new completion time sample */
#define SM_GET_TIMING(METRICS, INDEX, TIMER) \
	mc.AddSample(METRICS[INDEX].mh, TIMER.getElapsedTimeMs());
/** Acquire a new EXC reconfigured sample */
#define SM_ADD_SCHED(METRICS, INDEX, COUNT) \
	mc.AddSample(METRICS[INDEX].mh, COUNT);

namespace br = bbque::res;
namespace bu = bbque::utils;
namespace bp = bbque::plugins;
namespace po = boost::program_options;

namespace bbque {

/* Definition of metrics used by this module */
MetricsCollector::MetricsCollection_t
SchedulerManager::metrics[SM_METRICS_COUNT] = {
	//----- Event counting metrics
	SM_COUNTER_METRIC("runs",	"Scheduler executions count"),
	SM_COUNTER_METRIC("comp",	"Scheduler completions count"),
	SM_COUNTER_METRIC("start",	"START count"),
	SM_COUNTER_METRIC("reconf",	"RECONF count"),
	SM_COUNTER_METRIC("migrate","MIGRATE count"),
	SM_COUNTER_METRIC("migrec",	"MIGREC count"),
	SM_COUNTER_METRIC("block",	"BLOCK count"),
	//----- Timing metrics
	SM_SAMPLE_METRIC("time",	"Scheduler execution t[ms]"),
	SM_SAMPLE_METRIC("period",	"Scheduler activation period t[ms]"),
	//----- Couting statistics
	SM_SAMPLE_METRIC("avg.start",	"Avg START per schedule"),
	SM_SAMPLE_METRIC("avg.reconf",	"Avg RECONF per schedule"),
	SM_SAMPLE_METRIC("avg.migrec",	"Avg MIGREC per schedule"),
	SM_SAMPLE_METRIC("avg.migrate",	"Avg MIGRATE per schedule"),
	SM_SAMPLE_METRIC("avg.block",	"Avg BLOCK per schedule"),

};


SchedulerManager & SchedulerManager::GetInstance() {
	static SchedulerManager rs;
	return rs;
}

SchedulerManager::SchedulerManager() :
	am(ApplicationManager::GetInstance()),
	mc(bu::MetricsCollector::GetInstance()),
	sched_count(0) {
	std::string opt_policy;

	//---------- Get a logger module
	bp::LoggerIF::Configuration conf(SCHEDULER_MANAGER_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "RS: Logger module creation FAILED\n");
		assert(logger);
	}

	logger->Debug("Starting resource scheduler...");

	//---------- Loading module configuration
	ConfigurationManager & cm = ConfigurationManager::GetInstance();
	po::options_description opts_desc("Resource Scheduler Options");
	opts_desc.add_options()
		("SchedulerManager.policy",
		 po::value<std::string>
		 (&opt_policy)->default_value(BBQUE_DEFAULT_SCHEDULER_MANAGER_POLICY),
		 "The name of the optimization policy to use")
		;
	po::variables_map opts_vm;
	cm.ParseConfigurationFile(opts_desc, opts_vm);

	//---------- Load the required optimization plugin
	std::string opt_namespace(SCHEDULER_POLICY_NAMESPACE".");
	logger->Debug("Loading optimization policy [%s%s]...",
			opt_namespace.c_str(), opt_policy.c_str());
	policy = ModulesFactory::GetSchedulerPolicyModule(
			opt_namespace + opt_policy);
	if (!policy) {
		logger->Fatal("Optimization policy load FAILED "
			"(Error: missing plugin for [%s%s])",
			opt_namespace.c_str(), opt_policy.c_str());
		assert(policy);
	}

	//---------- Setup all the module metrics
	mc.Register(metrics, SM_METRICS_COUNT);

}

SchedulerManager::~SchedulerManager() {
}

#define SM_COLLECT_STATS(STATE) \
	count = am.AppsCount(ApplicationStatusIF::STATE);\
	SM_COUNT_EVENTS(metrics, SM_SCHED_ ## STATE, count);\
	SM_ADD_SCHED(metrics, SM_SCHED_AVG_ ## STATE, (double)count);

void
SchedulerManager::CollectStats() {
	uint16_t count;

	// Account for scheduling decisions
	SM_COLLECT_STATS(STARTING);
	SM_COLLECT_STATS(RECONF);
	SM_COLLECT_STATS(MIGREC);
	SM_COLLECT_STATS(MIGRATE);
	SM_COLLECT_STATS(BLOCKED);

}

SchedulerManager::ExitCode_t
SchedulerManager::Schedule() {
	ResourceAccounter &ra = ResourceAccounter::GetInstance();
	System &sv = System::GetInstance();
	SchedulerPolicyIF::ExitCode result;
	RViewToken_t svt;

	if (!policy) {
		logger->Crit("Resource scheduling FAILED (Error: missing policy)");
		assert(policy);
		return MISSING_POLICY;
	}

	// TODO add here proper tracing/monitoring events
	// for statistics collection

	// TODO here should be plugged a scheduling decision policy
	// Such a policy should decide whter a scheduling sould be run
	// or not, e.g. based on the kind of READY applications or considering
	// stability problems and scheduling overheads.
	// In case of a scheduling is not considered safe proper at this time,
	// a DELAYED exit code should be returned
	DB(logger->Warn("TODO: add scheduling activation policy"));

	// Call the current optimization plugin scheduling policy
	logger->Info("Resources scheduling, policy [%s]...",
			policy->Name());


	// Collecing execution metrics
	if (sched_count)
		SM_GET_TIMING(metrics, SM_SCHED_PERIOD, sm_tmr);
	sched_count++;

	// Account for actual scheduling runs
	SM_COUNT_EVENT(metrics, SM_SCHED_RUNS);

	// Reset timer for schedule execution time collection
	SM_RESET_TIMING(sm_tmr);

	result = policy->Schedule(sv, svt);
	if (result != SchedulerPolicyIF::SCHED_DONE) {
		logger->Error("Scheduliung policy [%s] failed",
				policy->Name());
		return FAILED;
	}

	// Clear the next AWM from the RUNNING Apps/EXC
	ClearRunningApps();

	// Set the scheduled resource view
	ra.SetScheduledView(svt);

	// Collecing execution metrics
	SM_GET_TIMING(metrics, SM_SCHED_TIME, sm_tmr);

	// Reset timer for schedule period time collection
	SM_RESET_TIMING(sm_tmr);

	// Account for scheduling completed
	SM_COUNT_EVENT(metrics, SM_SCHED_COMP);

	// Collect statistics on scheduling decisions
	CollectStats();

	return DONE;
}

void SchedulerManager::ClearRunningApps() {
	AppsUidMapIt apps_it;
	AppPtr_t papp;

	// Running Applications/EXC
	papp = am.GetFirst(ApplicationStatusIF::RUNNING, apps_it);
	for (; papp; papp = am.GetNext(ApplicationStatusIF::RUNNING, apps_it)) {

		// Commit a running state (this cleans the next AWM)
		am.RunningCommit(papp);
	}
}

} // namespace bbque

