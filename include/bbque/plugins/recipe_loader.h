/**
 *       @file  recipe_loader.h
 *      @brief  The interface for loading application recipes
 *
 * This defines the interface for loading recipe files describing applications
 * information such as AWM, priorities, etc.
 * @see Recipe for more details.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  31/03/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_RECIPE_LOADER_H_
#define BBQUE_RECIPE_LOADER_H_

#include <ctime>
#include <memory>

#include "bbque/app/application.h"

#define RECIPE_LOADER_NAMESPACE "rloader."

namespace ba = bbque::app;

namespace bbque { namespace plugins {

/**
 * @class RecipeLoaderIF
 *
 * @brief Basic interface for application recipe loading plugins.
 *
 * This is a required plugin.
 * Barbeque RTRM needs at least one RecipeLoader plugin to be registered in
 * order to work properly.
 * Indeed such plugin allows the retrieving of application resources usages
 * defined in the recipes, optional constraints, working modes values, and so
 * on...
 * @see Recipe for more details.
 */
class RecipeLoaderIF {

public:

	/**
	 * @enum Recipe load exit codes
	 */
	enum ExitCode_t {
		/** Load completed with success */
		RL_SUCCESS = 0,
		/** Partial load completed (i.e., some resources are missing) */
		RL_WEAK_LOAD,
		/** Recipe not found */
		RL_NOT_FOUND,
		/** Recipe wrong data and/or format */
		RL_FORMAT_ERROR,
		/** Loading aborted (i.e., a RTRM component is missing) */
		RL_ABORTED
	};

	/**
	 * @brief Load the recipe of the application
	 * @param app Pointer to the application using the recipe
	 * @param rname The recipe name. We expect to find the recipe in the
	 * path:<br><default-dir>/<i>rname</i>.recipe.
	 * @param recipe The recipe object to fill with the data to parse
	 * @return An exit code indicating the loading status
	 */
	virtual	ExitCode_t LoadRecipe(std::shared_ptr<ba::Application> app,
				std::string const & rname,
				std::shared_ptr<ba::Recipe> recipe)
		= 0;

	/**
	 * @brief The last modified time of the recipe
	 * @param path The relative path of the XML file containing the recipe
	 * @return A time_t object for timestamp comparison
	 */
	virtual std::time_t LastModifiedTime(std::string const & path) = 0;
};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_RECIPE_LOADER_H_

