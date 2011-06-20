/**
 *       @file  application.h
 *      @brief  Application descriptor
 *
 * This defines the application descriptor.
 * Such descriptor includes static and dynamic information upon application
 * execution. It embeds usual information about name, priority, user, PID
 * (could be different from the one given by OS) plus a reference to the
 * recipe object, the list of enabled working modes and resource constraints.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  05/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_APPLICATION_H
#define BBQUE_APPLICATION_H

#include <cassert>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bbque/app/application_conf.h"
#include "bbque/app/recipe.h"
#include "bbque/app/plugin_data.h"
#include "bbque/plugins/logger.h"

#define APPLICATION_NAMESPACE "ap"

using bbque::plugins::LoggerIF;

namespace bbque { namespace app {

// Forward declaration
class Application;

/** Shared pointer to Application object */
typedef std::shared_ptr<Application> AppPtr_t;
/** Shared pointer to Recipe object */
typedef std::shared_ptr<Recipe> RecipePtr_t;
/** Shared pointer to Constraint object */
typedef std::shared_ptr<Constraint> ConstrPtr_t;
/** Map of Constraints pointers, with the resource path as key*/
typedef std::map<std::string, ConstrPtr_t> ConstrMap_t;


/**
 * @class Application
 * @brief Application descriptor object
 *
 * When an application enter the RTRM it should specify sets of informations
 * as name, pid, priority,... working modes (resources requirements),
 * constraints. This very basic needs to let the RTRM doing policy-driven
 * choices upon resource assignments to the applications.
 */
class Application: public ApplicationConfIF {

public:

	/**
	 * @brief Constructor with parameters name and priority class
	 * @param name Application name
	 * @param user The user who has launched the application
	 * @param pid Process ID
	 * @param exc_id The ID of the Execution Context (assigned from the application)
	 */
	explicit Application(std::string const & name, AppPid_t pid, uint8_t exc_id);

	/**
	 * @brief Default destructor
	 */
	virtual ~Application();

	/**
	 * @see ApplicationStatusIF
	 */
	inline std::string const & Name() const {
		return name;
	}

	/**
	 * @brief Set the application name
	 * @param app_name The application name
	 */
	inline void SetName(std::string const & app_name) {
		name = app_name;
	}

	/**
	 * @brief Get the process ID of the application
	 * @return PID value
	 */
	inline AppPid_t Pid() const {
		return pid;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline uint8_t ExcId() const {
		return exc_id;
	}

	/**
	 * @brief Get a string ID for this Execution Context
	 * This method build a string ID according to this format:
	 * <PID>:<TASK_NAME>:<EXC_ID>
	 * @return String ID
	 */
	inline const char *StrId() const {
		return str_id;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	AppPrio_t Priority() const {
		return priority;
	}

	/**
	 * @see ApplicationConfIF
	 */
	void SetPriority(AppPrio_t prio);

	/**
	 * @see ApplicationConfIF
	 */
	ExitCode_t Enable();

	/**
	 * @see ApplicationConfIF
	 */
	ExitCode_t Disable();

	/**
	 * @brief This returns all the informations loaded from the recipe and
	 * stored in a specific object
	 * @return A shared pointer to the recipe object
	 */
	inline RecipePtr_t GetRecipe() {
		return recipe;
	}

	/**
	 * @brief Set the current recipe used by the application.
	 *
	 * The recipe should be loaded by the application manager using a
	 * specific plugin.
	 *
	 * @param recipe Recipe object shared pointer
	 * @param papp Shared pointer to the current Application.
	 *
	 * @note papp should be provided by ApplicationManager, which instances
	 * the new Application descriptor
	 */
	void SetRecipe(RecipePtr_t & recipe, AppPtr_t & papp);

	/**
	 * @see ApplicationStatusIF
	 */
	inline State_t State() const {
		return schedule.state;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline State_t PreSyncState() const {
		return schedule.preSyncState;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline SyncState_t SyncState() const {
		return schedule.syncState;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline AwmPtr_t const & CurrentAWM() const {
		return schedule.awm;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline AwmPtr_t const & NextAWM() const {
		return schedule.next_awm;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline AwmPtrList_t const * WorkingModes() {
		return &enabled_awms;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline AwmPtr_t const & LowValueAWM() {
		return enabled_awms.front();
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline AwmPtr_t const & HighValueAWM() {
		return enabled_awms.back();
	}

	/**
	 * @brief Get a working mode descriptor
	 * @param wmId Working mode ID
	 * @return The (enabled) working mode descriptor
	 *
	 * @note The working mode must come from the enabled list
	 */
	AwmPtr_t GetWorkingMode(uint16_t wmId);

	/**
	 * @see ApplicationConfIF
	 */
	ExitCode_t ScheduleRequest(AwmPtr_t const & awm,
			UsagesMapPtr_t & resource_set, RViewToken_t vtok = 0);

	/**
	 * @see ApplicationConfIF
	 */
	ExitCode_t ScheduleCommit();

	/**
	 * @brief Define a specific resource constraint. If exists yet it
	 * is overwritten. This could bring to have some AWM disabled.
	 *
	 * @param res_path Resource path
	 * @param type The constraint type (@see ContraintType)
	 * @param value The constraint value
	 * @return An error code (@see ExitCode_t)
	 */
	ExitCode_t SetConstraint(std::string const & res_path,
			Constraint::BoundType_t type, uint32_t value);

	/**
	 * @brief Remove a constraint upon a specific resource.
	 * This could bring to have some AWM re-enabled.
	 *
	 * @param res_path A pointer to the resource object
	 * @param type The constraint type (@see ContraintType)
	 * @return An error code (@see ExitCode)
	 */
	ExitCode_t RemoveConstraint(std::string const & res_path,
			Constraint::BoundType_t type);

	/**
	 * @brief Terminate this EXC by releasing all resources.
	 *
	 * This method requires to mark the EXC as terminated and to prepare the
	 * ground for releasing all resources as soon as possible. Due to
	 * asynchronous nature of this event and the activation of Optimized and
	 * Synchronizer, a valid reference to the object is granted to be keept
	 * alive until all of its users have terminated.
	 */
	ExitCode_t Terminate();


private:


	/** The logger used by the application */
	LoggerIF  *logger;

	/** The application name */
	std::string name;

	/** The user who has launched the application */
	std::string user;

	/** The PID assigned from the OS */
	AppPid_t pid;

	/** The ID of this Execution Context */
	uint8_t exc_id;

	/** The application string ID */
	char str_id[16];

	/** A numeric priority value */
	AppPrio_t priority;

	/** Current scheduling informations */
	SchedulingInfo_t schedule;

	/** The mutex to serialize access to scheduling info */
	std::mutex schedule_mtx;

	/**
	 * Recipe pointer for the current application instance.
	 * At runtime we could manage many instances of the same application using
	 * different recipes.
	 */
	RecipePtr_t recipe;

	/** Map containing all the working modes */
	AwmMap_t working_modes;

	/** List of pointers to enabled working modes */
	AwmPtrList_t enabled_awms;

	/** Resource contraints asserted */
	ConstrMap_t constraints;

	/**
	 * @brief Init working modes by reading the recipe
	 * @param papp Pointer to the current Application, allocated by
	 * ApplicationManager
	 */
	void InitWorkingModes(AppPtr_t & papp);

	/**
	 * @brief Init constraints by reading the recipe
	 *
	 * The method reads the "static" constraints on resources.
	 */
	void InitConstraints();

	/**
	 * @brief Whenever a constraint is set or removed, the method is called in
	 * order to check if there are some working mode to disable or re-enable.
	 *
	 * @param res_path Resource path upon which the constraint has been
	 * asserted or from which has been removed
	 * @param type Constraint type (lower or upper bound)
	 * @param value The value of the constraint
	 */
	void WorkingModesEnabling(std::string const & res_path,
			Constraint::BoundType_t type, uint64_t value);

	/**
	 * @brief Update the application state and sync state
	 *
	 * @param state the new application state
	 * @param sync the new synchronization state (SYNC_NONE by default)
	 */
	void SetState(State_t state, SyncState_t sync = SYNC_NONE);

	/**
	 * @brief Reset the state to the pre-synchronization state
	 */
	void ResetState();

	/**
	 * @brief Update the application synchronization state
	 */
	void SetSyncState(SyncState_t sync);

	/**
	 * @brief Request a synchronization of this application into the specied
	 * state.
	 *
	 * @param sync the new synchronization state (SYNC_NONE by default)
	 */
	ExitCode_t RequestSync(SyncState_t sync);

	/**
	 * @brief Configure this application to switch to the specified AWM
	 */
	ExitCode_t Reschedule(AwmPtr_t const & awm);

	/**
	 * @brief Configure this application to release resources.
	 */
	ExitCode_t Unschedule();

	/**
	 * @brief Verify if a synchronization is required to move into the
	 * specified AWM.
	 *
	 * The method is called only if the Application is currently RUNNING.
	 * Compare the WorkingMode specified with the currently used by the
	 * Application. Compare the Resources set with the one binding the
	 * resources of the current WorkingMode, in order to check if the
	 * Application is going to run using processing elements from the same
	 * clusters used in the previous execution step.
	 *
	 * @param awm the target working mode
 	 *
	 * @return One of the following values:
	 * - RECONF: Application is being scheduled for using PEs from the same
	 *   clusters, but with a different WorkingMode.
	 * - MIGRATE: Application is going to run in the same WorkingMode, but
	 *   it’s going to be moved onto a different clusters set.
	 * - MIGREC: Application changes completely its execution profile. Both
	 *   WorkingMode and clusters set are different from the previous run.
	 * - SYNC_NONE: Nothing changes. Application is going to run in the same
	 *   operating point.
	 *
	 */
	SyncState_t SyncRequired(AwmPtr_t const & awm);

	/**
	 * @brief Notify the EXC being run
	 *
	 * This method is called by ScheduleCommit once an EXC has been
	 * synchronized to the RUNNING state. It update the current EXC state to
	 * RUNNING
	 */
	ExitCode_t SetRunning();

	/**
	 * @brief Notify the EXC being blocked
	 *
	 * This method is called by ScheduleCommit once an EXC has been
	 * synchronized to the BLOCKED state. It update the current EXC state to
	 * either READY or FINISHED according to what happened meanwhile. If the
	 * EXC has been simply disabled, it becomes READY, while insted
	 */
	ExitCode_t SetBlocked();

};

} // namespace app

} // namespace bbque

#endif // BBQUE_APPLICATION_H

