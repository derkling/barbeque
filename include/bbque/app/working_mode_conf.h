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

#ifndef BBQUE_WORKING_MODE_CONF_IF_H
#define BBQUE_WORKING_MODE_CONF_IF_H

#include "bbque/app/working_mode_status.h"

using bbque::res::ResID_t;

namespace bbque { namespace app {


class WorkingModeConfIF;
/** Shared pointer to the class here defined */
typedef std::shared_ptr<WorkingModeConfIF> AwmCPtr_t;


/**
 * @brief Working Mode configureation interfaace
 *
 * This is the working mode interface used for runtime information updating
 */
class WorkingModeConfIF: public WorkingModeStatusIF {

public:

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
	 * @param bid An optional binding identifier
	 *
	 * @return WM_SUCCESS if the binding has been correctly performed.
	 * WM_ERR_RSRC_NAME if the resource name specified is not correct, and
	 * WM_RSRC_MISS_BIND if the resulting binding is incomplete.
	 *
	 * @note Use RSRC_ID_ANY if you want to bind the resource without care
	 * about its ID.
	 */
	virtual ExitCode_t BindResource(std::string const & rsrc_name,
			ResID_t src_ID, ResID_t dst_ID, uint8_t bid = 0) = 0;

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
