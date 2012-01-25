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

#ifndef BBQUE_APPLICATION_CONF_IF_H_
#define BBQUE_APPLICATION_CONF_IF_H_

#include "bbque/app/application_status.h"
#include "bbque/rtlib.h"

namespace bbque { namespace res {
/** Numeric value used as token for the resource views */
typedef size_t RViewToken_t;

struct ResourceUsage;
/** Shared pointer to ResourceUsage object */
typedef std::shared_ptr<ResourceUsage> UsagePtr_t;
/** Map of ResourceUsage descriptors. Key: resource path */
typedef std::map<std::string, UsagePtr_t> UsagesMap_t;
/** Constant pointer to the map of ResourceUsage descriptors */
typedef std::shared_ptr<UsagesMap_t> UsagesMapPtr_t;
}}

using bbque::res::UsagesMapPtr_t;
using bbque::res::RViewToken_t;

namespace bbque { namespace app {

/**
 * @brief Interface to configure application status
 *
 * This defines the interfaces for updating runtime information of the
 * application as priority, scheduled status and next working mode
 */
class ApplicationConfIF: public ApplicationStatusIF {

public:

	/**
	 * @brief Set the priority of the application
	 */
	virtual void SetPriority(AppPrio_t prio) = 0;

	/**
	 * @brief Enable the application for resources assignment
	 *
	 * A newly created application is disabled by default, thus it will not be
	 * considered for resource scheduling until it is enabled.
	 */
	virtual ExitCode_t Enable() = 0;

	/**
	 * @brief Disabled the application for resources assignment
	 *
	 * A disabled application will not be considered for resources scheduling.
	 */
	virtual ExitCode_t Disable() = 0;

	/**
	 * @brief Request to re-schedule this application into a new configuration
	 *
	 * The Optimizer call this method when an AWM is selected for this
	 * application to verify if it could be scheduled, i.e. bound resources
	 * are available, and eventually to update the application status.
	 *
	 * First the application verify resources availability. If the quality and
	 * amount of required resources could be satisfied, the application is
	 * going to be re-scheduled otherwise, it is un-scheduled.
	 *
	 * @param awm Next working mode scheduled for the application
	 * @param tok The token referencing the resources state view
	 * @param bid An optional identifier for the resource binding
	 *
	 * @return The method return an exit code representing the decision taken:
	 * APP_WM_ACCEPTED if the specified working mode can be scheduled for
	 * this application, APP_WM_REJECTED if the working mode cannot not be
	 * scheduled. If the application is currently disabled this call returns
	 * always APP_DISABLED.
	 */
	virtual ExitCode_t ScheduleRequest(AwmPtr_t const & awm, RViewToken_t tok,
			uint8_t bid = 0) = 0;

	/**
	 * @brief Terminate this EXC by releasing all resources.
	 *
	 * This method requires to mark the EXC as terminated and to prepare the
	 * ground for releasing all resources as soon as possible. Due to
	 * asynchronous nature of this event and the activation of Optimized and
	 * Synchronizer, a valid reference to the object is granted to be keept
	 * alive until all of its users have terminated.
	 */
	virtual ExitCode_t Terminate() = 0;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_APPLICATION_CONF_IF_H_
