/**
 *       @file  sasb_schedpol.h
 *      @brief  The SASB synchronization policy
 *
 * This defines a dynamic C++ plugin which implements the "Starvation Avoidance
 * State Based" (SASB) heuristic for EXCc synchronizaiton.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#ifndef BBQUE_SASB_SYNCPOL_H_
#define BBQUE_SASB_SYNCPOL_H_

#include "bbque/plugins/synchronization_policy.h"
#include "bbque/plugins/logger.h"
#include "bbque/plugins/plugin.h"

#include "bbque/utils/timer.h"
#include "bbque/utils/metrics_collector.h"

#include <cstdint>

#define SYNCHRONIZATION_POLICY_NAME "sasb"

using bbque::utils::Timer;
using bbque::utils::MetricsCollector;

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

/**
 * @class SasbSyncPol
 * @brief The Random resource scheduler heuristic registered as a dynamic C++
 * plugin.
 */
class SasbSyncPol : public SynchronizationPolicyIF {

public:

//----- static plugin interface

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	static void * Create(PF_ObjectParams *);

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	static int32_t Destroy(void *);

	~SasbSyncPol();

//----- Scheduler Policy module interface

	char const * Name();

	ApplicationStatusIF::SyncState_t GetApplicationsQueue(
			bbque::SystemView & system, bool restart = false);

	bool DoSync(AppPtr_t papp);

	ExitCode_t CheckLatency(AppPtr_t papp, SyncLatency_t latency);

	SyncLatency_t EstimatedSyncTime();
	
private:

	typedef enum syncState {
		STEP10 = 0,
		STEP21,
		STEP22,
		STEP23,
		STEP31,
		STEP32,
		STEP33,
		STEP40
	} syncState_t;

	uint8_t status;

	/**
	 * @brief System logger instance
	 */
	LoggerIF *logger;

	/**
	 * @brief Keep track of the best estimation for the sync latency
	 */
	SyncLatency_t max_latency;

	/** The metrics collector */
	MetricsCollector & mc;

	/** The set of metrics collected by this plugin */
	typedef enum SyncPolMetrics {
		//----- Event counting metrics
		SM_SASB_RUNS = 0,
		//----- Timing metrics
		SM_SASB_TIME_START,
		SM_SASB_TIME_RECONF,
		SM_SASB_TIME_MIGREC,
		SM_SASB_TIME_MIGRATE,
		SM_SASB_TIME_BLOCKED,

		SM_METRICS_COUNT
	} SyncMgrMetrics_t;

	/** The High-Resolution timer used for profiling */
	Timer sm_tmr;

	/** The collection of metrics used by this plugin */
	static MetricsCollector::MetricsCollection_t metrics[SM_METRICS_COUNT];

	/**
	 * @brief   The plugins constructor
	 * Plugins objects could be build only by using the "create" method.
	 * Usually the PluginManager acts as object
	 * @param   
	 * @return  
	 */
	SasbSyncPol();

	ApplicationStatusIF::SyncState_t step1(bbque::SystemView & system);

	ApplicationStatusIF::SyncState_t step2(bbque::SystemView & system);

	ApplicationStatusIF::SyncState_t step3(bbque::SystemView & system);

	ApplicationStatusIF::SyncState_t step4(bbque::SystemView & system);

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_SASB_SYNCPOL_H_

