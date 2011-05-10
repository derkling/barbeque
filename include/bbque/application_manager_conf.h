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
 * @brief "Updating" interface for ApplicationManager
 */
class ApplicationManagerConfIF: public ApplicationManagerStatusIF {

public:

	/**
	 * @brief The entry point for the applications requiring Barbeque.
	 * @param name The application name
	 * @param prio Static priority
	 * @param pid PID of the application (assigned from the OS)
	 * @param exc_id The ID of the Execution Context (assigned from the application)
	 * @param rpath Recipe location
	 * @param weak_load If true, the recipe accept a "weak load".
	 * It means that if a resource is missing, Barbeque can look for a
	 * substituting resource taken from a higher level in the resource tree.
	 *
	 * @return An error code
	 */
	virtual plugins::RecipeLoaderIF::ExitCode_t StartApplication(
			std::string const & name, uint16_t prio, pid_t pid, uint8_t exc_id,
			std::string const & rpath, bool weak_load) = 0;

};

} // namespace bbque

#endif // BBQUE_APPLICATION_MANAGER_CONF_IF_H_

