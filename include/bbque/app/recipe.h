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
#include "bbque/app/constraints.h"
#include "bbque/app/plugin_data.h"
#include "bbque/plugins/logger.h"

#define RECIPE_NAMESPACE "rcp"

using bbque::plugins::LoggerIF;

namespace bbque { namespace app {

// Forward declarations
class Application;
class WorkingMode;

/** Shared pointer to Application descriptor  */
typedef std::shared_ptr<Application> AppPtr_t;
/** Shared pointer to Working Mode descriptor */
typedef std::shared_ptr<WorkingMode> AwmPtr_t;
/** Map of shared pointers to WorkingMode */
typedef std::map<uint8_t, AwmPtr_t> AwmMap_t;
/** Shared pointer to Constraint object */
typedef std::shared_ptr<ResourceConstraint> ConstrPtr_t;
/** Map of Constraints pointers, with the resource path as key*/
typedef std::map<std::string, ConstrPtr_t> ConstrMap_t;

/**
 * @class Recipe
 *
 * Applications need the set of working mode definitions, plus (optionally)
 * constraints) and other information in order to be managed by Barbeque.
 * Such stuff is stored in a Recipe object. Each application can use more than
 * one recipe, but a single instance must specify the one upon which base its
 * execution.
 */
class Recipe: public PluginsData {

friend class Application;

public:

	/*
	 * @brief Constructor
	 * @param path The pathname for retrieving the recipe
	 */
	Recipe(std::string const & name);

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
	 * @brief Get the priority parsed from the recipe
	 * @return Priority value
	 */
	inline uint8_t GetPriority() const {
		return priority;
	}

	/**
	 * @brief Set the priority value
	 */
	void SetPriority(uint8_t prio) {
		priority = prio;
	}

	/**
	 * @brief Insert an application working mode
	 * @param app Application owning the working mode
	 * @param id Working mode ID
	 * @param name Working mode descripting name
	 * @param value The user QoS value of the working mode
	 */
	AwmPtr_t & AddWorkingMode(uint8_t id, std::string const & name,
					float value);

	/**
	 * @brief Remove an application working mode inserted
	 * @param id Working mode ID
	 */
	inline void RemoveWorkingMode(uint8_t id) {
		working_modes.erase(id);
	}

	/**
	 * @brief Return an application working mode object by specifying
	 * its identifying name
	 * @param id Working mode ID
	 * @return A shared pointer to the application working mode searched
	 */
	AwmPtr_t WorkingMode(uint8_t id);

	/**
	 * @brief All the working modes defined into the recipe
	 * @return A vector containing all the working modes
	 */
	inline AwmMap_t const & WorkingModesAll() {
		return working_modes;
	}

	/**
	 * @brief Add a constraint to the static constraint map
	 *
	 * Lower bound assertion: AddConstraint(<resource_path>, value, 0);
	 * Upper bound assertion: AddConstraint(<resource_path>, 0, value);
	 *
	 * If a bound value has been specified previously, take the the greater
	 * between the previous and the current one.
	 *
	 * @param rsrc_path Resource path to constrain.
	 * @param lb Lower bound value
	 * @param ub Upper bound value
	 */
	void AddConstraint(std::string const & rsrc_path, uint64_t lb, uint64_t ub);

	/**
	 * @brief Get all the static constraints
	 * @return The map of static constraints
	 */
	inline ConstrMap_t const & ConstraintsAll() {
		return constraints;
	}

private:

	/** The logger used by the application */
	LoggerIF  *logger;

	/**
	 * Starting from a common recipes root directory, each recipe file
	 * should be named "<application>_<recipe-qualifier>.recipe".
	 * If the recipe is not stored in files but in a database system (i.e.,
	 * GSettings) the following attribute can be an information string useful
	 * for retrieving the recipe.
	 */
	std::string pathname;

	/** Priority */
	uint8_t priority;

	/** The complete set of working modes descriptors defined in the recipe */
	AwmMap_t working_modes;

	/** Static constraints included in the recipe */
	ConstrMap_t constraints;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_RECIPE_H_

