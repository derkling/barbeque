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

#define SCHEDULER_POLICY_NAME "yamca"

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

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

	~YamcaSchedPol();

//----- Scheduler Policy module interface

	char const * Name();

	SchedulerPolicyIF::ExitCode_t
		Schedule(bbque::SystemView const & system);

private:

	/**
	 * @brief System logger instance
	 */
	LoggerIF *logger;

	/**
	 * @brief   The plugins constructor
	 * Plugins objects could be build only by using the "create" method.
	 * Usually the PluginManager acts as object
	 * @param   
	 * @return  
	 */
	YamcaSchedPol();

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_YAMCA_SCHEDPOL_H_

