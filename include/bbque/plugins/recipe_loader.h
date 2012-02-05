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

#ifndef BBQUE_RECIPE_LOADER_H_
#define BBQUE_RECIPE_LOADER_H_

#include <ctime>
#include <memory>

#include "bbque/app/recipe.h"

#define RECIPE_LOADER_NAMESPACE "bq.rl."

using bbque::app::Recipe;

namespace bbque { namespace plugins {

/**
 * @brief Basic interface for application recipe loading plugins.
 *
 * This defines the interface for loading recipe files describing applications
 * information such as AWM, priorities, etc.  Barbeque RTRM needs at least one
 * RecipeLoader plugin to be registered in order to work properly.  Indeed
 * such plugin allows the retrieving of application resources usages defined
 * in the recipes, optional constraints, working modes values, and so on.
 * @see Recipe for more details.
 * @note This is a required plugin.
 */
class RecipeLoaderIF {

public:

	/**
	 * @brief Recipe load exit codes
	 */
	enum ExitCode_t {
		//--- Success load
		/** Load completed with success */
		RL_SUCCESS = 0,
		/** Partial load completed (i.e., some resources are missing) */
		RL_WEAK_LOAD,

		//--- Failed load
		/** Recipe load failed for some reason (generic error) */
		RL_FAILED,
		/** Load completed with success */
		RL_MISSING_LOADER,
		/** Recipe not found */
		RL_NOT_FOUND,
		/** Recipe wrong data and/or format */
		RL_FORMAT_ERROR,
		/** Loading aborted (i.e., a RTRM component is missing) */
		RL_ABORTED
	};

	/**
	 * @brief Load the recipe of the application
	 * @param rname The recipe name. We expect to find the recipe in the
	 * path:<br><default-dir>/<i>recipe_name</i>.recipe.
	 * @param recipe The recipe object to fill with the data to parse
	 * @return An exit code indicating the loading status
	 */
	virtual	ExitCode_t LoadRecipe(std::string const & rname,
			std::shared_ptr<Recipe> recipe)	= 0;

	/**
	 * @brief The last modified time of the recipe
	 * @param recipe_name The recipe name
	 * @return A time_t object for timestamp comparison
	 */
	virtual std::time_t LastModifiedTime(std::string const & recipe_name) = 0;
};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_RECIPE_LOADER_H_
