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
	virtual AppPtr_t StartApplication(
			std::string const & name, AppPid_t pid, uint8_t exc_id,
			std::string const & recipe, app::AppPrio_t prio, bool weak_load) = 0;

	/**
	 * @brief Exit point for applications
	 *
	 * Stop the execution of an application scheduled with Barbeque RTRM.
	 *
	 * @param pid The process ID of the application
	 */
	virtual void StopApplication(AppPid_t pid) = 0;

	/**
	 * @brief Exit point for execution contexts of applications
	 *
	 * Stop the execution of an execution context, belonging to a given
	 * application scheduled with Barbeque RTRM.
	 *
	 * @param pid The process ID of the application
	 * @param exc_id The internal ID of the execution context
	 */
	virtual void StopApplication(AppPid_t pid, uint8_t exc_id) = 0;

};

} // namespace bbque

#endif // BBQUE_APPLICATION_MANAGER_CONF_IF_H_

