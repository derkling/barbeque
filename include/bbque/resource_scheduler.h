/**
 *       @file  resource_scheduler.h
 *      @brief  The resource scheduler component of Barbque
 *
 * This module defines the Barbeque interface to resource scheduling policies.
 * The core framework view only methods exposed by this component, which is on
 * charge to find, select and load a proper optimization policy, to run it when
 * required by new events happening (e.g. applications starting/stopping,
 * resources state/availability changes) and considering its internal
 * configurabile policy.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  05/25/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef BBQUE_RESOURCE_SCHEDULER_H_
#define BBQUE_RESOURCE_SCHEDULER_H_

#include "bbque/config.h"
#include "bbque/plugin_manager.h"

#include "bbque/plugins/logger.h"
#include "bbque/plugins/scheduler_policy.h"

using bbque::plugins::LoggerIF;
using bbque::plugins::SchedulerPolicyIF;

#ifdef BBQUE_DEBUG
# define BBQUE_DEFAULT_RESOURCE_SCHEDULER_POLICY "random"
#else
# define BBQUE_DEFAULT_RESOURCE_SCHEDULER_POLICY ""
#endif


namespace bbque {

/**
 * @class ResourceScheduler
 * @brief The class implementing the glue code to bind Barbeque with resource
 * scheduling policies.
 */
class ResourceScheduler {

public:

	typedef enum ExitCode {
		DONE = 0,
		MISSING_POLICY,
		FAILED,
		DELAYED
	} ExitCode_t;

	/**
	 * @brief Get a reference to the resource scheduler
	 * The ResourceScheduler is a singleton class, an instance to this class
	 * could be obtained by the resource manager in order to access the
	 * optimization policy services.
	 * @return  a reference to the ResourceScheduler singleton instance
	 */
	static ResourceScheduler & GetInstance();

	/**
	 * @brief Clean-up the optimization policy and releasing current resource
	 * scheduler
	 */
	~ResourceScheduler();

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

	/**
	 * @brief Build a new instance of the resource scheduler
	 */
	ResourceScheduler();




};

} // namespace bbque

#endif // BBQUE_RESOURCE_SCHEDULER_H_

