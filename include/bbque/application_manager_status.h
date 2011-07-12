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
typedef std::map<AppUid_t, AppPtr_t > AppsUidMap_t;

/**
 * Map containing shared pointers to Application descriptors, where the key is
 * the application PID
 */
typedef std::multimap<AppPid_t, AppPtr_t > AppsMap_t;

/**
 * An entry of the Application Map
 */
typedef std::pair<AppPid_t, AppPtr_t> AppsMapEntry_t;

/**
 * An entry of the UIDS Map
 */
typedef std::pair<AppUid_t, AppPtr_t> UidsMapEntry_t;

/**
 * @class ApplicationManagerStatusIF
 * @brief "Read-only" interface for ApplicationManager status
 */
class ApplicationManagerStatusIF {

public:

	/**
	 * @enum Exit code to return
	 */
	enum ExitCode_t {
		/** Success */
		AM_SUCCESS = 0,
		/** Application Execution Context not found */
		AM_EXC_NOT_FOUND,
		/** Execution of a method interrupted by an unexpected state in an
		 * internal data structure state */
		AM_DATA_CORRUPT,
		/** Method forced to exit */
		AM_ABORT
	};

	/**
	 * @brief Retrieve regietered applications map
	 *
	 * @return A pointer to the Map of all applications registered to the RTRM
	 */
	virtual AppsUidMap_t const * Applications() const = 0;

	/**
	 * @brief Retrieve registered applications with the specified priority
	 *
	 * @param prio The priority value
	 * @return A pointer to the map of applications with the request priority
	 */
	virtual AppsUidMap_t const * Applications(app::AppPrio_t prio) const = 0;

	/**
	 * @brief Retrieve applications with the specified state
	 *
	 * This method allows to retrive a map of applications into a specified
	 * state.
	 *
	 * @param sched_state The scheduling state
	 *
	 * @return A pointer to the map of applications with the requested state
	 */
	virtual AppsUidMap_t const * Applications(
			app::Application::State_t state) const = 0;

	/**
	 * @brief Retrieve applications with the specified synchronization state
	 *
	 * This method allows to retrive a map of applications into a specified
	 * synchronization state
	 *
	 * @param sched_state The synchronization state
	 *
	 * @return A pointer to the map of applications with the requested
	 * synchronization state
	 */
	virtual AppsUidMap_t const * Applications(
			app::Application::SyncState_t sync_state) const = 0;

	/**
	 * @brief Retrieve an application descriptor (shared pointer) by PID and
	 * Excution Context
	 * @param pid Application PID
	 * @param exc_id Execution Contetx ID
	 */
	virtual AppPtr_t const GetApplication(AppPid_t pid,
			uint8_t exc_id) = 0;

	/**
	 * @brief Retrieve an application descriptor (shared pointer) by UID
	 *
	 * @param uid Application UID
	 */
	virtual AppPtr_t const GetApplication(AppUid_t uid) const = 0;

	/**
	 * @brief Lowest application priority
	 * @return The maximum integer value for the (lowest) priority level
	 */
	virtual app::AppPrio_t LowestPriority() const = 0;

	/**
	 * @brief Dump a logline to report all applications status
	 */
	virtual void PrintStatusReport() const = 0;

};

} // namespace bbque

#endif // BBQUE_APPLICATION_MANAGER_STATUS_IF_H_

