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
	virtual void SetValue(float value) = 0;

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
	virtual ExitCode_t AddOverheadInfo(uint8_t dest_awm_id, double time) = 0;

	/**
	 * @brief Bind resource usages to system resources descriptors
	 *
	 * Resource paths taken from the recipes use IDs that do not care about
	 * the real system resource IDs registered by Barbeque. Thus a binding
	 * must be solved in order to make the request of resources satisfiables.
	 *
	 * The method takes the resource name we want to bind (i.e. "cluster"),
	 * the source resource ID (optional), i.e. the ID specified in the recipe,
	 * and the destination ID related to the system resource to bind. Then it
	 * substitutes "cluster+[source ID]" with "cluster+[dest ID]" and get the
	 * list of descriptors (bindings) returned by ResourceAccounter.
	 *
	 * If a binding on the same resource is performed again, before the
	 * current binding has been set for scheduling, the behavior is the one
	 * explained below:
	 *
	 * BindResource("cluster", 0, 1);   // Recp ID 0 -> Sys ID 1
	 * BindResource("cluster", 1, 2);   //  Sys ID 1 -> Sys ID 2 = Recp ID 0
	 *
	 * Therefore... BEWARE! To undo the previous binding (the whole map) @see
	 * ClearSchedResourceBinding().
	 *
	 * If the source ID is left blank, the method will bind ALL the
	 * resource path containing "cluster" or "clusterN" to the descriptor
	 * referenced by such path, with the destination resource ID in
	 * the system.
	 *
	 * As result a map of ResourceUsage objects is built. It defines the set
	 * of resources to allocate to the application, according to the
	 * assignment of the working mode.
	 *
	 * @param rsrc_name The resource name we want to bind
	 * @param src_ID Recipe resource name ID
	 * @param dst_ID System resource name ID
	 *
	 * @return WM_SUCCESS if the binding has been correctly performed.
	 * WM_ERR_RSRC_NAME if the resource name specified is not correct, and
	 * WM_RSRC_MISS_BIND if the resulting binding is incomplete.
	 *
	 * @note Use RSRC_ID_ANY if you want to bind the resource without care
	 * about its ID.
	 */
	virtual ExitCode_t BindResource(std::string const & rsrc_name,
			ResID_t src_ID, ResID_t dst_ID) = 0;

	/**
	 * @brief Clear the resource binding to schedule
	 *
	 * The method cancel the current resource binding "to schedule", i.e. the
	 * map of resource usages before builds through BindResource(), and not
	 * already set.
	 */
	void ClearSchedResourceBinding();
};

} // namespace app

} // namespace bbque

#endif // BBQUE_WORKING_MODE_CONF_IF_H

