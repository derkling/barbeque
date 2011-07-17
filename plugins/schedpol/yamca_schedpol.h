/**
 *       @file  yamca_schedpol.h
 *      @brief  The YaMCA resource scheduler (dynamic plugin)
 *
 * This defines a dynamic C++ plugin which implements the YaMCA resource
 * scheduler heuristic.
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

#ifndef BBQUE_YAMCA_SCHEDPOL_H_
#define BBQUE_YAMCA_SCHEDPOL_H_

#include "bbque/plugins/scheduler_policy.h"
#include "bbque/plugins/plugin.h"

#include <cstdint>

#include "bbque/system_view.h"
#include "bbque/app/application_status.h"
#include "bbque/app/working_mode_status.h"
#include "bbque/res/resource_accounter_status.h"

using namespace bbque::app;
using namespace bbque::res;

using bbque::app::AppPtr_t;
using bbque::app::AwmPtr_t;

#define SCHEDULER_POLICY_NAME "yamca"

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

/** The scheduling entity*/
typedef std::pair<AppPtr_t, AwmPtr_t> SchedEntity_t;
/** Map for ordering the scheduling entities */
typedef std::multimap<double, SchedEntity_t> SchedEntityMap_t;

// Forward declaration
class LoggerIF;

/**
 * @class YamcaSchedPol
 * @brief The YaMCA resource scheduler heuristic registered as a dynamic C++
 * plugin.
 */
class YamcaSchedPol : public SchedulerPolicyIF {

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

	/**
	 * @brief Destructor
	 */
	~YamcaSchedPol();

//----- Scheduler Policy module interface

	/**
	 * @see SchedulerPolicyIF
	 */
	char const * Name();

	/**
	 * @see ScheduerPolicyIF
	 */
	SchedulerPolicyIF::ExitCode_t
		Schedule(bbque::SystemView const & system);

private:

	/** System logger instance */
	LoggerIF *logger;

	/** Resource accounter instance */
	ResourceAccounter & rsrc_acct;

	/** Token for accessing a resources view */
	RViewToken_t rsrc_view_token;

	/** A counter used for getting always a new clean resources view */
	uint32_t tok_counter;

	/** Number of clusters on the platform	 */
	uint64_t num_clusters;

	/**
	 * @brief The plugins constructor
	 *
	 * Plugins objects could be build only by using the "create" method.
	 * Usually the PluginManager acts as object.
	 */
	YamcaSchedPol();

	/**
	 * @brief Get a token for accessing a clean resource view
	 * @return The token returned by ResourceAccounter.
	 */
	ExitCode_t InitResourceView();

	/**
	 * @brief Schedule applications from a priority queue
	 * @param apps Map of applications having the same priority
	 *
	 * @return @see ExitCode_t
	 */
	ExitCode_t SchedulePrioQueue(AppsUidMap_t const * apps);

	/**
	 * @brief Scheduling entities ordering
	 *
	 * For each application create a scheduling entity made by the pair
	 * Application-WorkingMode and place it in a map (ordered by the metrics
	 * value computed)
	 *
	 * @param sched_map Multimap for scheduling entities ordering
	 * @param apps Map of applications to schedule
	 * @param cl_id The current cluster for the clustered resources
	 */
	ExitCode_t OrderSchedEntity(SchedEntityMap_t & sched_map,
			AppsUidMap_t const * apps, int cl_id);

	/**
	 * @brief Metrics of all the AWMs of an Application
	 *
	 * @param sched_map Multimap for scheduling entities ordering
	 * @param papp Application to evaluate
	 * @param cl_id The current cluster for the clustered resources
	 */
	ExitCode_t InsertWorkingModes(SchedEntityMap_t & sched_map,
			AppPtr_t const & papp, int cl_id);

	/**
	 * @brief Schedule the entities
	 *
	 * Fore each application pick the next working mode to schedule
	 *
	 * @param sched_map Multimap for scheduling entities ordering
	 */
	void SelectWorkingModes(SchedEntityMap_t & sched_map);

	/**
	 * @brief Metrics computation
	 *
	 * For each scheduling entity computes a metrics used to define an order.
	 * Such order is used to pick the working mode to set to the application.
	 *
	 * @param papp Application to evaluate
	 * @param wm Working mode to evaluate
	 * @param cl_id The current cluster for the clustered resources
	 * @param metrics Metrics value to return
	 * @return @see ExitCode_t
	 */
	ExitCode_t MetricsComputation(AppPtr_t const & papp,
			AwmPtr_t const & wm, int cl_id, double & metrics);

	/**
	 * @brief Get resources contention level
	 *
	 * Return a parameter for the evaluation of the scheduling metrics. In
	 * particuar it catches the impact of resource request on the total
	 * availability.
	 *
	 * @param wm The working mode containing the resource requests
	 * @param cl_id The current cluster for the clustered resources
	 * @param cont_level The contention level value to return
	 * @return @see ExitCode_t
	 */
	ExitCode_t GetContentionLevel(AwmPtr_t const & wm, int cl_id,
			double & cont_level);

	/**
	 * @brief Compute the resource contention level
	 *
	 * @param rsrc_usages Map of resource usages to bind
	 * @param cont_level The contention level value to return
	 * @return @see ExitCode_t
	 */
	ExitCode_t ComputeContentionLevel(UsagesMapPtr_t const & rsrc_usages,
			double & cont_level);

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_YAMCA_SCHEDPOL_H_

