/**
 *       @file  application_manager_conf.h
 *      @brief  ApplicationManager runtime updating interface
 *
 * This provides the interface of the application manager for updating
 * applications runtime information.
 * Currently the only feature provided by this interface is the application
 * recipe loading.
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

#ifndef BBQUE_APPLICATION_MANAGER_CONF_IF_H_
#define BBQUE_APPLICATION_MANAGER_CONF_IF_H_

#include "bbque/application_manager_status.h"
#include "bbque/plugins/recipe_loader.h"

namespace bbque {

/**
 * @class ApplicationManagerConfIF
 *
 * The interface exposes the ApplicationManager methods for applications
 * lifecycle manipulation.
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

