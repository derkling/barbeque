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
	 * @param dest_awm Destination working mode
	 * @param time Time spent in switching to destination working mode
	 * @return A reference to the TransitionOverheads just created
	 */
	virtual ExitCode_t AddOverheadInfo(std::string const & dest_awm,
			double time) = 0;

	/**
	 * @brief Bind resource usages paths to system resources
	 *
	 * Resource paths taken from the recipes use IDs that don't care about the
	 * real system resource IDs registered by Barbeque. Thus a binding must be
	 * solved in order to make the request of resources satisfiables.
	 *
	 * The method takes the resource name we want to bind (i.e. "cluster"),
	 * the ID of the destination system resource, and the source resource ID
	 * (optional). Then it substitutes "cluster+[source ID]" with
	 * "cluster+[dest ID]" and get the descriptor (list of) returned by
	 * ResourceAccounter.
	 *
	 * If the source ID is left blank, the method will bind ALL the
	 * resource path containing "cluster" or "clusterN" to the descriptor
	 * referenced by such path, with the destination resource ID in
	 * the system.
	 *
	 * @param rsrc_name The resource name we want to bind
	 * @param dst_id System resource name destination ID
	 * @param src_id Recipe resource name source ID
	 * @param rsrc_path_unbound A resource path left to bind
	 * @return An exit code (@see ExitCode_t)
	 */
	ExitCode_t BindResources(std::string const & rsrc_name,
			int32_t dst_id, int32_t src_id = -1,
			const char * rsrc_path_unbound = NULL);

};

} // namespace app

} // namespace bbque

#endif // BBQUE_WORKING_MODE_CONF_IF_H

