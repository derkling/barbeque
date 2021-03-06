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

#ifndef BBQUE_SCHEDULER_MANAGER_H_
#define BBQUE_SCHEDULER_MANAGER_H_

#include "bbque/config.h"
#include "bbque/plugin_manager.h"
#include "bbque/application_manager.h"

#include "bbque/utils/timer.h"
#include "bbque/utils/metrics_collector.h"

#include "bbque/plugins/logger.h"
#include "bbque/plugins/scheduler_policy.h"

using bbque::plugins::LoggerIF;
using bbque::plugins::SchedulerPolicyIF;

using bbque::utils::Timer;
using bbque::utils::MetricsCollector;

#define SCHEDULER_MANAGER_NAMESPACE "bq.sm"

#ifdef BBQUE_DEBUG
# define BBQUE_DEFAULT_SCHEDULER_MANAGER_POLICY "random"
#else
# define BBQUE_DEFAULT_SCHEDULER_MANAGER_POLICY "yams"
#endif


namespace bbque {

/**
 * @brief The glue logic for resources scheduling.
 * @ingroup sec05_sm
 *
 * This module defines the BarbequeRTRM interface to resource scheduling policies.
 * The core framework view only methods exposed by this component, which is on
 * charge to find, select and load a proper optimization policy, to run it when
 * required by new events happening (e.g. applications starting/stopping,
 * resources state/availability changes) and considering its internal
 * configurabile policy.

 */
class SchedulerManager {

public:

	typedef enum ExitCode {
		DONE = 0,
		MISSING_POLICY,
		FAILED,
		DELAYED
	} ExitCode_t;

	/**
	 * @brief Get a reference to the resource scheduler
	 * The SchedulerManager is a singleton class, an instance to this class
	 * could be obtained by the resource manager in order to access the
	 * optimization policy services.
	 * @return  a reference to the SchedulerManager singleton instance
	 */
	static SchedulerManager & GetInstance();

	/**
	 * @brief Clean-up the optimization policy and releasing current resource
	 * scheduler
	 */
	~SchedulerManager();

	/**
	 * @brief Run a new resource scheduling optimization
	 * The barbeque main control loop calls this method when a new resource
	 * scheduling is required. However, it is up to the internal policy defined
	 * by this class to decide wheter a new reconfiguration should run or not.
	 * This policy is tunable with a set of configuration options exposed by
	 * the barbeque configuration file.
	 * However, in any case this method return an exit code which could be used
	 * to understad the resource scheduler descision (e.g. optimization done,
	 * optimization post-poned, ...)
	 */
	ExitCode_t Schedule();

private:

	/**
	 * @brief The logger used by the resource manager.
	 */
	LoggerIF *logger;

	/**
	 * @brief The currently used optimization policy
	 */
	SchedulerPolicyIF *policy;

	ApplicationManager & am;
	MetricsCollector & mc;

	/**
	 * @brief The number of synchronization cycles
	 */
	uint32_t sched_count;

	/**
	 * @brief The collection of metrics generated by this module
	 */
	typedef enum ScheMgrMetrics {
		//----- Event counting metrics
		SM_SCHED_RUNS = 0,
		SM_SCHED_COMP,
		SM_SCHED_STARTING,
		SM_SCHED_RECONF,
		SM_SCHED_MIGREC,
		SM_SCHED_MIGRATE,
		SM_SCHED_BLOCKED,
		//----- Timing metrics
		SM_SCHED_TIME,
		SM_SCHED_PERIOD,
		//----- Couting statistics
		SM_SCHED_AVG_STARTING,
		SM_SCHED_AVG_RECONF,
		SM_SCHED_AVG_MIGREC,
		SM_SCHED_AVG_MIGRATE,
		SM_SCHED_AVG_BLOCKED,

		SM_METRICS_COUNT
	} SchedMgrMetrics_t;

	/** The High-Resolution timer used for profiling */
	Timer sm_tmr;

	static MetricsCollector::MetricsCollection_t metrics[SM_METRICS_COUNT];

	/**
	 * @brief Build a new instance of the resource scheduler
	 */
	SchedulerManager();

	/**
	 * @brief Collect statistics on schedule results
	 */
	void CollectStats();

	/**
	 * @brief Clear next AWM in RUNNING Applications/EXC
	 */
	void ClearRunningApps();

};

} // namespace bbque

#endif // BBQUE_SCHEDULER_MANAGER_H_
