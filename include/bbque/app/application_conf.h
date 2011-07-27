/**
 *       @file  application_conf.h
 *      @brief  Application updating status interface
 *
 * This defines the interfaces for updating runtime information of the
 * application as priority, scheduled status and next working mode
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  04/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
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
 * @class ApplicationConfIF
 * @brief Provide interfaces to update runtime application information of the
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
	 * application to verify if it could be scheduled, i.e. resources are
	 * available, and eventually to update the application status.
	 *
	 * First the application verify resources availability. If the quality and
	 * amount of required resources could be satisfied, the application is
	 * going to be re-scheduled otherwise, it is un-scheduled.
	 *
	 * @param awm Next working mode scheduled for the application
	 * @param resource_set The map of resources on which bind the working mode
	 * @param tok The token referencing the resources state view
	 *
	 * @return The method return an exit code representing the decision taken:
	 * APP_WM_ACCEPTED if the specified working mode could be scheduled for
	 * this application, APP_WM_REJECTED if the working mode could not beed
	 * scheduled. If the application is currently disabled this call returns
	 * always APP_DISABLED.
	 */
	virtual ExitCode_t ScheduleRequest(AwmPtr_t const & awm,
			UsagesMapPtr_t & resource_set, RViewToken_t tok) = 0;

	/**
	 * @brief Set a constraint on the working modes
	 *
	 * It's important to clearly explain the behavior of the following API.
	 * To set a lower bound means that all the AWMs having an ID lesser than
	 * the one specified in the method call will be disabled. Thus they will
	 * not considered by the scheduler. Similarly, but accordingly to the
	 * reverse logics, if an upper bound is set: all the working modes with ID
	 * greater to the one specified will be disabled.
	 * An "exact value" constraint instead means that the scheduler should
	 * consider only the working mode specified.
	 *
	 * Whenever a constraint of the same type occurs, the previous constraint
	 * is replaced. This means that if the application sets a lower bound, a
	 * second lower bound assertion will "overwrite" the firsts. The same for
	 * upper and exact value cases.
	 *
	 * @constraint @see RTLIB_Constraint
	 * @return APP_SUCCESS for success. APP_WM_NOT_FOUND if the working mode
	 * id is not correct. APP_ABORT if 'add' field of RTLIB_Constraint
	 * argument is unset (false).
	 */
	virtual ExitCode_t SetWorkingModeConstraint(RTLIB_Constraint & constraint)
		= 0;

	/**
	 * @brief Remove a constraint on the working modes
	 *
	 * The method simply removes the bound type specified as argument.
	 * In case of "exact value" bound, the method will enable all the working
	 * modes, whatever are the current boundaries.
	 *
	 * If an exact value bound is currently set to working mode X, and the
	 * method is called to remove an upper bound, the resulting enabled AWMs
	 * would be = [X, last].
	 * Converserly if the lower bound is removed the result would be =
	 * [first, X].
	 */
	virtual void ClearWorkingModeConstraint(RTLIB_ConstraintType & cstr_type)
		= 0;

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

