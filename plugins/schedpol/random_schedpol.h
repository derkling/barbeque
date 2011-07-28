/**
 *       @file  random_schedpol.h
 *      @brief  The Random resource scheduler (dynamic plugin)
 *
 * This defines a dynamic C++ plugin which implements the Random resource
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

#ifndef BBQUE_RANDOM_SCHEDPOL_H_
#define BBQUE_RANDOM_SCHEDPOL_H_

#include "bbque/app/application.h"
#include "bbque/plugins/scheduler_policy.h"
#include "bbque/plugins/plugin.h"

#include <cstdint>

#define SCHEDULER_POLICY_NAME "random"

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

using bbque::app::AppPtr_t;
using bbque::res::RViewToken_t;

namespace bbque { namespace plugins {

// Forward declaration
class LoggerIF;

/**
 * @class RandomSchedPol
 * @brief The Random resource scheduler heuristic registered as a dynamic C++
 * plugin.
 */
class RandomSchedPol : public SchedulerPolicyIF {

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

	~RandomSchedPol();

//----- Scheduler Policy module interface

	char const * Name();

	SchedulerPolicyIF::ExitCode_t
		Schedule(bbque::SystemView & sv);

private:

	/**
	 * @brief System logger instance
	 */
	LoggerIF *logger;

	/**
	 * 
	 */
	RViewToken_t ra_view;
	
	/**
	 * @brief Random Number Generator engine used for AWM selection
	 */
	std::uniform_int_distribution<uint16_t> dist;

	/**
	 * @brief   The plugins constructor
	 * Plugins objects could be build only by using the "create" method.
	 * Usually the PluginManager acts as object
	 * @param   
	 * @return  
	 */
	RandomSchedPol();

	/**
	 * @brief Randonly select an AWM for the application
	 */
	void ScheduleApp(AppPtr_t papp);
};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_RANDOM_SCHEDPOL_H_

