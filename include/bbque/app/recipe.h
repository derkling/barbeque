/**
 *       @file  recipe.h
 *      @brief  class Recipe for Application information storage
 *
 * This defines the class for object storing Application information the RTRM
 * needs for managing its execution.
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

#ifndef BBQUE_RECIPE_H_
#define BBQUE_RECIPE_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "bbque/app/plugin_data.h"

namespace bbque { namespace app {

// Forward declarations
class Application;
class WorkingMode;

/** Type definition of a shared pointer to a Application descriptor object */
typedef std::shared_ptr<Application> AppPtr;

/** Type definition of a shared pointer to a Working Mode descriptor object */
typedef std::shared_ptr<WorkingMode> AwmPtr;

/**
 * @class Recipe
 *
 * Applications need the set of working mode definitions, plus (optionally)
 * constraints) and other information in order to be managed by Barbeque.
 * Such stuff is stored in a Recipe object. Each application can use more than
 * one recipe, but a single instance must specify the one upon which base its
 * execution.
 */
class Recipe {

public:

	/*
	 * @brief Constructor
	 * @param path The pathname for retrieving the recipe
	 */
	Recipe(std::string path):
		pathname(path) {
	}

	/**
	 * @brief Destructor
	 */
	virtual ~Recipe();

	/**
	 * @brief Get the path of the recipe
	 * @return The recipe path string (or an info for retrieving it)
	 */
	inline std::string const & Path() {
		return pathname;
	}

	/**
	 * @brief Insert an application working mode
	 * @param app Application descriptor pointer
	 * @param name Working mode name
	 * @param value The QoS value
	 */
	void AddWorkingMode(AppPtr app, std::string const & name, uint8_t value);

	/**
	 * @brief Remove an application working mode inserted
	 * @param name Working mode name
	 */
	void RemoveWorkingMode(std::string const & name);

	/**
	 * @brief Return an application working mode object by specifying
	 * its identifying name
	 * @param name Working mode name
	 * @return A shared pointer to the application working mode searched
	 */
	AwmPtr WorkingMode(std::string const & name);

	/**
	 * @brief All the working modes defined into the recipe
	 * @return A vector containing all the working modes
	 */
	inline std::vector<AwmPtr> const & WorkingModesAll() {
		return working_modes;
	}

private:

	/**
	 * Starting from a common recipes root directory, each recipe file
	 * should be named "<application>_<recipe-qualifier>.recipe".
	 * If the recipe is not stored in files but in a database system (i.e.,
	 * GSettings) the following attribute can be an information string useful
	 * for retrieving the recipe.
	 */
	std::string pathname;

	/** The complete set of working modes descriptors defined in the recipe */
	std::vector<AwmPtr> working_modes;

	/**
	 * @brief Internal method used for working mode searches.
	 * @return An iterator pointing to the working mode object found in
	 * the <tt>working_modes</tt> vector
	 */
	std::vector<AwmPtr>::const_iterator _GetWorkingModeIterator(
			std::string	const & name);

};

} // namespace app

} // namespace bbque

#endif // BBQUE_RECIPE_H_

