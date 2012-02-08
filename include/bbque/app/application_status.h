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

#ifndef BBQUE_APPLICATION_STATUS_IF_H_
#define BBQUE_APPLICATION_STATUS_IF_H_

#include <cassert>
#include <list>
#include <mutex>
#include <string>

#include "bbque/app/recipe.h"
#include "bbque/utils/attributes_container.h"

using bbque::utils::AttributesContainer;


#define BBQUE_UID_SHIFT 5
#define BBQUE_UID_MASK 0x1F


namespace bbque { namespace app {


/** The application identifier type */
typedef uint32_t AppPid_t;

/** The application UID type */
typedef uint32_t AppUid_t;

/** The application priority type */
typedef uint16_t AppPrio_t;

/** List of WorkingMode pointers */
typedef std::list<AwmPtr_t> AwmPtrList_t;


/**
 * @brief Provide interfaces to query application information
 *
 * This defines the interface for querying Application runtime information:
 * name, priority, current  working mode and scheduled state, next working
 * mode and scheduled state, and the list of all the active working modes
 */
class ApplicationStatusIF: public AttributesContainer {

public:

	/**
	 * @enum ExitCode_t
	 * Error codes returned by methods
	 */
	enum ExitCode_t {
		/** Success */
		APP_SUCCESS = 0,
		/** Application being DISABLED */
		APP_DISABLED,
		/** Null recipe object passed */
		APP_RECP_NULL,
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
		/** The list of enabled working modes has changed */
		APP_WM_ENAB_CHANGED,
		/** The list of enabled working modes has not changed */
		APP_WM_ENAB_UNCHANGED,
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

	/**
	 * @brief Required synchronization action
	 *
	 * Once a reconfiguration for an EXC has been scheduled, one of these
	 * synchronization state is entered. Vice versa, if no reconfiguration are
	 * required, the SYNC_NONE (alias SYNC_STATE_COUNT) is assigned to the
	 * EXC.
	 */
	typedef enum SyncState {
	// NOTE These values should be reported to match (in number and order)
	//      those defined by the RTLIB::RTLIB_ExitCode.
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
		/** The mutex to serialize access to scheduling info */
		std::recursive_mutex mtx;
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
	 * PID:TASK_NAME:EXC_ID
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
	virtual State_t State() = 0;

	/**
	 * @brief Get the pre-synchronization state
	 */
	virtual State_t PreSyncState()= 0;

	/**
	 * @brief Check if this EXC is currently DISABLED
	 */
	virtual bool Disabled()= 0;

	/**
	 * @brief Check if this EXC is currently READY of RUNNING
	 */
	virtual bool Active()= 0;

	/**
	 * @brief Check if this EXC is currently in SYNC state
	 */
	virtual bool Synching()= 0;

	/**
	 * @brief Check if this EXC is being BLOCKED
	 */
	virtual bool Blocking() = 0;

	/**
	 * @brief Get the synchronization state
	 */
	virtual SyncState_t SyncState() = 0;

	inline char const *SyncStateStr() {
		return syncStateStr[SyncState()];
	}

	/**
	 * @brief Get the current working mode
	 * @return A shared pointer to the current application working mode
	 */
	virtual AwmPtr_t const & CurrentAWM() = 0;

	/**
	 * @brief Get next working mode to switch in when the application is
	 * re-scheduld
	 * @return A shared pointer to working mode descriptor (optimizer
	 * interface)
	 */
	virtual AwmPtr_t const & NextAWM() = 0;

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
