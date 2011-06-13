/**
 *       @file  working_mode_conf.h
 *      @brief  "Updating" interface for WorkingMode runtime status
 *
 * This defines the interface for updating WorkingMode at runtime by other
 * RTRM components, i.e. the Optimizer.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  01/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_WORKING_MODE_CONF_IF_H
#define BBQUE_WORKING_MODE_CONF_IF_H

#include "bbque/app/working_mode_status.h"

using bbque::res::ResID_t;

namespace bbque { namespace app {

/**
 * @class WorkingModeConfIF
 *
 * This is the working mode interface used for runtime information updating
 */
class WorkingModeConfIF: public WorkingModeStatusIF {

public:

	/**
	 * @brief Set the QoS value associated to the working mode
	 *
	 * The value is viewed as a kind of satisfaction level, from the user
	 * perspective, about the execution of the application.
	 * This method has thinked to allow (optional) changes of WM values, as a
	 * consequence of changes of scheduling policy.
	 *
	 * @param value The QoS value of the working mode
	 */
	virtual void SetValue(uint16_t value) = 0;

	/**
	 * @brief Init a new transition overheads descriptor
	 *
	 * Create a new object TransitionOverheads managing the overhead
	 * information in switching to 'dest_awm' working mode.
	 *
	 * @param dest_awm_id Destination working mode ID
	 * @param time Time spent in switching to destination working mode
	 * @return A reference to the TransitionOverheads just created
	 */
	virtual ExitCode_t AddOverheadInfo(uint16_t dest_awm_id, double time) = 0;

	/**
	 * @brief Set a resource binding for the resource usages
	 *
	 * Use BindResources to get a map of ResourceUsage objects. These objects
	 * contain the amount of resource requested (value) and a list of system
	 * resource descriptors.
	 *
	 * @param usages_bind A map of ResourceUsage objects with solved binds
	 * @return WM_SUCCESS, or WM_RSRC_MISS_BIND	if some binds are missing
	 */
	ExitCode_t SetResourceBinding(UsagesMapPtr_t & usages_bind);
};

} // namespace app

} // namespace bbque

#endif // BBQUE_WORKING_MODE_CONF_IF_H

