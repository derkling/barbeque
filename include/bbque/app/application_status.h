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

#include <cassert>
#include <list>
#include <string>

#include "bbque/app/plugin_data.h"

/** The application identifier */
typedef uint32_t AppPid_t;

/** The application UID */
typedef uint32_t AppUid_t;

#define BBQUE_UID_SHIFT 5
#define BBQUE_UID_MASK 0x1F

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
		/** The working mode is schedulable */
		APP_WM_ACCEPTED,
		/** The working mode is not schedulable */
		APP_WM_REJECTED,
		/** Method forced to exit */
		APP_ABORT
	};

	/**
	 * @brief A possible application state
	 *
	 * This is the set of possible state an application could be.
	 * TODO: add complete explaination of each state, detailing how each state
	 * could be entered, when and which other Barbeque module trigger the
	 * transition.
	 */
	typedef enum State {
		/** Registered within Barbeque but currently disabled */
		DISABLED = 0,
		/** Ready to be scheduled */
		READY,
		/** The application must be reconfigured */
		SYNC,
		/** Running */
		RUNNING,
		/** Regular exit */
		FINISHED,

		/** This must alwasy be the last entry */
		STATE_COUNT
	} State_t;

	typedef enum SyncState {
		/** The application is entering the system */
		STARTING = 0,
		/** Must change working mode */
		RECONF ,
		/** Must migrate and change working mode */
		MIGREC,
		/** Must migrate into another cluster */
		MIGRATE,
		/** Must be blocked becaus of resource are not more available */
		BLOCKED,
		/** The applications is exiting the system */
		TERMINATE,

		/** This must alwasy be the last entry */
		SYNC_STATE_COUNT
	} SyncState_t;

#define SYNC_NONE SYNC_STATE_COUNT


	/**
	 * @struct SchedulingInfo_t
	 *
	 * Application scheduling informations.
	 * The scheduling of an application is characterized by a pair of
	 * information: the state (@see State_t), and the working mode
	 * choosed by the scheduler/optimizer module.
	 */
	struct SchedulingInfo_t {
		/** The current scheduled state */
		State_t state;
		/** The state before a sync has been required */
		State_t preSyncState;
		/** The current synchronization state */
		SyncState_t syncState;
		/** The current application working mode */
		AwmPtr_t awm;
		/** The next scheduled application working mode */
		AwmPtr_t next_awm;
		/** Overloading of operator != for structure comparisons */
		inline bool operator!=(SchedulingInfo_t const &other) const {
			return ((this->state != other.state) ||
					(this->preSyncState != other.preSyncState) ||
					(this->syncState != other.syncState) ||
					(this->awm != other.awm));
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
	 * @brief Get the UID of the current application
	 */
	inline AppUid_t Uid() const {
		return (Pid() << BBQUE_UID_SHIFT) + ExcId();
	};

	/**
	 * @brief Get the UID of an application given its PID and EXC
	 */
	static inline AppUid_t Uid(AppPid_t pid, uint8_t exc_id) {
		return (pid << BBQUE_UID_SHIFT) + exc_id;
	};

	/**
	 * @brief Get the PID of an application given its UID
	 */
	static inline AppPid_t Pid(AppUid_t uid) {
		return (uid >> BBQUE_UID_SHIFT);
	};

	/**
	 * @brief Get the EID of an application given its UID
	 */
	static inline uint8_t Eid(AppUid_t uid) {
		return (uid & BBQUE_UID_MASK);
	};

	/**
	 * @brief Get a string ID for this Execution Context
	 * This method build a string ID according to this format:
	 * <PID>:<TASK_NAME>:<EXC_ID>
	 * @return String ID
	 */
	virtual const char *StrId() const = 0;

	inline static char const *StateStr(State_t state) {
		assert(state < STATE_COUNT);
		return stateStr[state];
	}

	inline static char const *SyncStateStr(SyncState_t state) {
		assert(state < SYNC_STATE_COUNT+1);
		return syncStateStr[state];
	}


	/**
	 * @brief Get the priority associated to
	 * @return The priority value
	 */
	virtual AppPrio_t Priority() const = 0;

	/**
	 * @brief Get the schedule state
	 * @return The current scheduled state
	 */
	virtual State_t State() const = 0;

	/**
	 * @brief Get the synchronization state
	 */
	virtual SyncState_t SyncState() const = 0;

	/**
	 * @brief Get the current working mode
	 * @return A shared pointer to the current application working mode
	 */
	virtual AwmPtr_t const & CurrentAWM() const = 0;

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

private:

	/**
	 * @brief Verbose application state names
	 */
	static char const *stateStr[STATE_COUNT];

	/**
	 * @brief Verbose synchronization state names
	 */
	static char const *syncStateStr[SYNC_STATE_COUNT+1];

};

} // namespace app

} // namespace bbque

#endif // BBQUE_APPLICATION_STATUS_IF_H_

