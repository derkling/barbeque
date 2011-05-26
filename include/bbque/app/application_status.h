/**
 *       @file  application_status.h
 *      @brief  Read-only interface for Application runtime status
 *
 * This defines the interface for querying Application runtime information:
 * name, priority, current  working mode and scheduled state, next working
 * mode and scheduled state, and the list of all the active working modes
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

#ifndef BBQUE_APPLICATION_STATUS_IF_H_
#define BBQUE_APPLICATION_STATUS_IF_H_

#include <list>
#include <string>

#include "bbque/app/plugin_data.h"

/** The application identifier */
typedef uint32_t AppPid_t;


namespace bbque { namespace app {

/** The application priotity */
typedef uint16_t AppPrio_t;

class WorkingMode;
typedef std::shared_ptr<WorkingMode> AwmPtr_t;

/** List of WorkingMode pointers */
typedef std::list<AwmPtr_t> AwmPtrList_t;


/**
 * @class ApplicationStatusIF
 * @brief Provide interfaces to query application information
 */
class ApplicationStatusIF: public PluginsData {

public:

	/**
	 * @enum ExitCode_t
	 * Error codes returned by methods
	 */
	enum ExitCode_t {
		/** Success */
		APP_SUCCESS = 0,
		/** Application working mode not found */
		APP_WM_NOT_FOUND,
		/** Resource not found */
		APP_RSRC_NOT_FOUND,
		/** Constraint not found */
		APP_CONS_NOT_FOUND,
		/** The working mode is not schedulable */
		APP_WM_REJECTED,
		/** Method forced to exit */
		APP_ABORT
	};

	/**
	 * @enum ScheduleFlag_t
	 *
	 * This defines flags describing a schedule state.
	 * Suffixes (S, T) have just a code readability purpose. "S" means
	 * "stable" and "T" is "transitional".
	 * The semantic behind a transitional state implies that the scheduling
	 * should be finalized moving the application into a stable state.
	 * Transitional states are useful to make more explicit the
	 * scheduler/optimizer decisions, and eventually to trigger some overheads
	 * statistics data collection.
	 */
	enum ScheduleFlag_t {
		/** Registered within Barbeque but currently disabled */
		DISABLED = 0,
		/** Ready to be scheduled */
		READY,
		/** Must change working mode */
		RECONF,
		/** Must migrate into another cluster */
		MIGRATE,
		/** Must migrate and change working mode */
		MIGREC,
		/** Running */
		RUNNING,
		/** Waiting for an event or resource */
		BLOCKED,
		/** Regular exit */
		FINISHED
	};

	/**
	 * @struct SchedulingInfo_t
	 *
	 * Application scheduling informations.
	 * The scheduling of an application is characterized by a pair of
	 * information: the state (@see ScheduleFlag_t), and the working mode
	 * choosed by the scheduler/optimizer module.
	 */
	struct SchedulingInfo_t {
		/** A schedule state */
		ScheduleFlag_t state;
		/** A pointer to an application working mode */
		AwmPtr_t awm;
		/** Overloading of operator != for structure comparisons */
		inline bool operator!=(SchedulingInfo_t const &other) const {
			return ((this->state != other.state) || (this->awm != other.awm));
		};
	};

	/**
	 * @brief Get the name of the application
	 * @return The name string
	 */
	virtual std::string const & Name() const = 0;

	/**
	 * @brief Get the process ID of the application
	 * @return PID value
	 */
	virtual AppPid_t Pid() const = 0;

	/**
	 * @brief Get the ID of this Execution Context
	 * @return PID value
	 */
	virtual uint8_t ExcId() const = 0;

	/**
	 * @brief Get a string ID for this Execution Context
	 * This method build a string ID according to this format:
	 * <PID>:<TASK_NAME>:<EXC_ID>
	 * @return String ID
	 */
	virtual const char *StrId() const = 0;

	/**
	 * @brief Get the priority associated to
	 * @return The priority value
	 */
	virtual AppPrio_t Priority() const = 0;

	/**
	 * @brief Get the current schedule state
	 * @return The current scheduled state
	 */
	virtual ScheduleFlag_t CurrentState() const = 0;

	/**
	 * @brief Get the current working mode
	 * @return A shared pointer to the current application working mode
	 */
	virtual AwmPtr_t const & CurrentAWM() const = 0;

	/**
	 * @brief Get next schedule state
	 * @return Next schedule state
	 */
	virtual ScheduleFlag_t NextState() const = 0;

	/**
	 * @brief Get next working mode to switch in when the application is
	 * re-scheduld
	 * @return A shared pointer to working mode descriptor (optimizer
	 * interface)
	 */
	virtual AwmPtr_t const & NextAWM() const = 0;

	/**
	 * @brief The enabled working modes
	 * @return All the schedulable working modes of the application
	 */
	virtual AwmPtrList_t const * WorkingModes() = 0;

	/**
	 * @brief The working mode with the lowest value
	 * @return A pointer to the working mode descriptor having the lowest
	 * value
	 */
	virtual AwmPtr_t const & LowValueAWM() = 0;

	/**
	 * @brief The working mode with the highest value
	 * @return A pointer to the working mode descriptor having the highest
	 * value
	 */
	virtual AwmPtr_t const & HighValueAWM() = 0;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_APPLICATION_STATUS_IF_H_

