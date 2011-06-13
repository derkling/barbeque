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
	inline State_t CurrentState() const {
		return curr_sched.state;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline AwmPtr_t const & CurrentAWM() const {
		return curr_sched.awm;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline State_t NextState() const {
		return next_sched.state;
	}

	/**
	 * @see ApplicationStatusIF
	 */
	inline AwmPtr_t const & NextAWM() const {
		return next_sched.awm;
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
	 * @see ApplicationStatusIF
	 */
	ExitCode_t SetNextSchedule(AwmPtr_t const & awm, RViewToken_t vtok = 0);

	ExitCode_t _SetNextSchedule(AwmPtr_t const & awm, RViewToken_t vtok = 0);
	void _SetState(State_t state);
	ExitCode_t _RequestSync(SyncState_t ss);
	SyncState_t _SyncRequired(AwmPtr_t const & awm, RViewToken_t vtok = 0);
	ExitCode_t _Reschedule(AwmPtr_t const & awm, RViewToken_t vtok = 0);
	ExitCode_t _Unschedule();

	/**
	 * @brief Update scheduled status and reconfiguration overheads data
	 *
	 * When the application manager receives a notify about a change of
	 * scheduling profile (state and working mode) of an application, it needs
	 * to update some internal structures, and set the new state in the
	 * application descriptor.
	 * The method set the new current state and forward to the application
	 * information about overheads occourred in the reconfiguration phase.
	 *
	 * @param time The time measured/estimated in the reconfiguration process.
	 */
	void UpdateScheduledStatus(double time);

	/**
	 * @brief Stop the application execution
	 *
	 * Finalize the end of the execution.
	 */
	ExitCode_t StopExecution();

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

private:

	static char const *StateStr[STATE_COUNT];

	static char const *SyncStateStr[SYNC_STATE_COUNT];

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
	SchedulingInfo_t curr_sched;

	/** Next scheduled status */
	SchedulingInfo_t next_sched;

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

};

} // namespace app

} // namespace bbque

#endif // BBQUE_APPLICATION_H

