/**
 *       @file  application_manager_status.h
 *      @brief  Application manager "read-only" status interface
 *
 * This defines the interface of the Application Manager component for
 * querying the application runtime information.
 * Currently we are interested in getting a specific application descriptor,
 * the lowest priority level managed, and maps of application descriptors,
 * even querying by scheduling status or priority level.
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

#ifndef BBQUE_APPLICATION_MANAGER_STATUS_IF_H_
#define BBQUE_APPLICATION_MANAGER_STATUS_IF_H_

#include "bbque/app/application.h"

namespace bbque {

/** A shared pointer to the Application descriptor */
typedef std::shared_ptr<app::Application> AppPtr_t;

/**
 * Map containing shared pointers to Application descriptors, where the key is
 * the application PID
 */
typedef std::multimap<uint32_t, AppPtr_t > AppsMap_t;

/**
 * An entry of the Application Map
 */
typedef std::pair<uint32_t, AppPtr_t> AppsMapEntry_t;

/**
 * @class ApplicationManagerStatusIF
 * @brief "Read-only" interface for ApplicationManager status
 */
class ApplicationManagerStatusIF {

public:

	/**
	 * @brief Retrieve all the applications which entered the resource
	 * manager
	 * @return A map of all applications
	 */
	virtual AppsMap_t const & Applications() const = 0;

	/**
	 * @brief Retrieve all the applications of a specific priority
	 * @param prio The priority value
	 * @return A map of applications of the request priority
	 */
	virtual AppsMap_t const & Applications(uint16_t prio) const = 0;

	/**
	 * @brief Retrieve all the applications in a specific scheduling state
	 * @param sched_state The scheduling state
	 * @return A map of applications in the schedule status request
	 */
	virtual AppsMap_t const & Applications(
			app::Application::ScheduleFlag_t sched_state) const = 0;

	/**
	 * @brief Retrieve an application descriptor (shared pointer) by PID and
	 * Excution Context
	 * @param pid Application PID
	 * @param exc_id Execution Contetx ID
	 */
	virtual AppPtr_t const GetApplication(uint32_t pid,
			uint32_t exc_id = 0) = 0;

	/**
	 * @brief Maximum integer value for the minimum application priority
	 */
	virtual uint16_t LowestPriority() const = 0;

};

} // namespace bbque

#endif // BBQUE_APPLICATION_MANAGER_STATUS_IF_H_

