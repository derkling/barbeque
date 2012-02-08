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

#ifndef BBQUE_APPLICATION_MANAGER_CONF_IF_H_
#define BBQUE_APPLICATION_MANAGER_CONF_IF_H_

#include "bbque/application_manager_status.h"
#include "bbque/plugins/recipe_loader.h"

namespace bbque {

/**
 * @brief The "Configuration" interface for the ApplicationManager.
 * @ingroup sec03_am
 *
 * This provides the interface of the ApplicationManager for updating
 * applications runtime information.
 * Currently the only feature provided by this interface is the application
 * recipe loading.

 */
class ApplicationManagerConfIF: public ApplicationManagerStatusIF {

public:

	/**
	 * @brief Entry point for applications
	 *
	 * @param name The application name
	 * @param pid PID of the application (assigned from the OS)
	 * @param exc_id The ID of the Execution Context (assigned from the application)
	 * @param recipe The name of the recipe to use for this application
	 * @param prio Static priority
	 * @param weak_load If true, the recipe accept a "weak load".
	 * It means that if a resource is missing, Barbeque can look for a
	 * substituting resource taken from a higher level in the resource tree.
	 *
	 * @return A pointer to the newly allocated application, NULL otherwise.
	 */
	virtual AppPtr_t CreateEXC(
			std::string const & name, AppPid_t pid, uint8_t exc_id,
			std::string const & recipe, app::AppPrio_t prio, bool weak_load) = 0;

	/**
	 * @brief Destroy all the EXCs of the specified application
	 *
	 * All the EXCs of this application will not be anymore scheduled by the
	 * Barbeque RTRM.
	 *
	 * @param pid The process ID of the application
	 */
	virtual ExitCode_t DestroyEXC(AppPid_t pid) = 0;

	/**
	 * @brief Destroy the specified EXC
	 *
	 * The specified EXC will not be anymore scheduled by the Barbeque RTRM.
	 *
	 * @param papp The pointer to the EXC to stop
	 */
	virtual ExitCode_t DestroyEXC(AppPtr_t papp) = 0;

	/**
	 * @brief Destroy the specified EXC
	 *
	 * The specified EXC will not be anymore scheduled by the Barbeque RTRM.
	 *
	 * @param pid The process ID of the application
	 * @param exc_id The internal ID of the execution context
	 */
	virtual ExitCode_t DestroyEXC(AppPid_t pid, uint8_t exc_id) = 0;

	/**
	 * @brief Set constraints asserted on the specified EXC
	 *
	 * The AWMs of the specified EXC are enabled/disabled based on the
	 * specified constraints.
	 *
	 * @param papp pointer to the application to enable
	 * @param constraints the array of constraints to be processed
	 * @param count the total number of constraints in the array
	 */
	virtual ExitCode_t SetConstraintsEXC(AppPtr_t papp,
			RTLIB_Constraint_t *constraints, uint8_t count) = 0;

	/**
	 * @brief Set constraints asserted on the specified EXC
	 *
	 * The AWMs of the specified EXC are enabled/disabled based on the
	 * specified constraints.
	 *
	 * @param pid the processi ID of the application
	 * @param exc_id the Execution Context ID to enable
	 * @param constraints the array of constraints to be processed
	 * @param count the total number of constraints in the array
	 */
	virtual ExitCode_t SetConstraintsEXC(AppPid_t pid, uint8_t exc_id,
			RTLIB_Constraint_t *constraints, uint8_t count) = 0;

	/**
	 * @brief Clear all the constraints asserted on the specified EXC
	 *
	 * If the specified EXC has some consraints, they are all removed and a
	 * scheduler request event is generated.
	 * @param papp pointer to the application to enable
	 */
	virtual ExitCode_t ClearConstraintsEXC(AppPtr_t papp) = 0;

	/**
	 * @brief Clear all the constratins asserted on the specified EXC
	 *
	 * If the specified EXC has some consraints, they are all removed and a
	 * scheduler request event is generated.
	 * @param pid the processi ID of the application
	 * @param exc_id the Execution Context ID to enable
	 */
	virtual ExitCode_t ClearConstraintsEXC(AppPid_t pid, uint8_t exc_id) = 0;

	/**
	 * @brief Set the Goal-Gap on the specified EXC
	 *
	 * Set the actual Goal-Gap for the currently selected AWMs of the
	 * specified EXC.
	 *
	 * @param papp pointer to the application to enable
	 * @param gap the current goal-gap value
	 */
	virtual ExitCode_t SetGoalGapEXC(AppPtr_t papp, uint8_t gap) = 0;

	/**
	 * @brief Set the Goal-Gap on the specified EXC
	 *
	 * Set the actual Goal-Gap for the currently selected AWMs of the
	 * specified EXC.
	 *
	 * @param pid the processi ID of the application
	 * @param exc_id the Execution Context ID to enable
	 * @param gap the current goal-gap value
	 */
	virtual ExitCode_t SetGoalGapEXC(AppPid_t pid, uint8_t exc_id,
			uint8_t gap) = 0;

	/**
	 * @brief Enable the EXC for resources scheduling
	 *
	 * Enable assignment of resources to the specified EXC.
	 * @param papp pointer to the application to enable
	 */
	virtual ExitCode_t EnableEXC(AppPtr_t papp) = 0;

	/**
	 * @brief Enable the EXC for resources scheduling
	 *
	 * Enable assignment of resources to the specified EXC.
	 * @param pid the processi ID of the application
	 * @param exc_id the Execution Context ID to enable
	 */
	virtual ExitCode_t EnableEXC(AppPid_t pid, uint8_t exc_id) = 0;

	/**
	 * @brief Disable the specified EXC for resources scheduling
	 *
	 * Disble assignment of resources to the specified EXC.
	 * @param papp pointer to the application to enable
	 */
	virtual ExitCode_t DisableEXC(AppPtr_t papp) = 0;

	/**
	 * @brief Disable the specified EXC for resources scheduling
	 *
	 * Disable assignment of resources to the specified EXC.
	 * @param pid the processi ID of the application
	 * @param exc_id the Execution Context ID to enable
	 */
	virtual ExitCode_t DisableEXC(AppPid_t pid, uint8_t exc_id) = 0;
};

} // namespace bbque

#endif // BBQUE_APPLICATION_MANAGER_CONF_IF_H_
