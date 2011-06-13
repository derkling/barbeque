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
	 * scheduled.
	 */
	virtual ExitCode_t ScheduleRequest(AwmPtr_t const & awm,
			UsagesMapPtr_t & resource_set, RViewToken_t tok = 0) = 0;

	// TODO: Remove this
	virtual ExitCode_t SetNextSchedule(AwmPtr_t const & awm,
			UsagesMapPtr_t & resource_set, RViewToken_t tok = 0) = 0;

	/**
	 * @brief Commit a previsously required re-scheduling request.
	 *
	 * @return APP_SUCCESS on successful update of internal data structures,
	 * APP_ABORT on errors.
	 */
	virtual ExitCode_t ScheduleCommit() = 0;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_APPLICATION_CONF_IF_H_

