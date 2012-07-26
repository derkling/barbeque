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

#ifndef BBQUE_RECIPE_H_
#define BBQUE_RECIPE_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "bbque/plugins/logger.h"
#include "bbque/utils/attributes_container.h"
#include "bbque/res/resource_constraints.h"

#define RECIPE_NAMESPACE "rcp"

// Maximum number of Application Working Modes manageable
#define MAX_NUM_AWM 	255

using bbque::plugins::LoggerIF;
using bbque::res::ResourceConstraint;
using bbque::utils::AttributesContainer;

namespace bbque { namespace app {

// Forward declarations
class Application;
class WorkingMode;

/** Shared pointer to Application descriptor  */
typedef std::shared_ptr<Application> AppPtr_t;
/** Shared pointer to Working Mode descriptor */
typedef std::shared_ptr<WorkingMode> AwmPtr_t;
/** Vector of shared pointer to WorkingMode*/
typedef std::vector<AwmPtr_t> AwmPtrVect_t;
/** Shared pointer to Constraint object */
typedef std::shared_ptr<ResourceConstraint> ConstrPtr_t;
/** Map of Constraints pointers, with the resource path as key*/
typedef std::map<std::string, ConstrPtr_t> ConstrMap_t;

/**
 * @brief An application recipe
 *
 * Applications need the set of working mode definitions, plus (optionally)
 * constraints) and other information in order to be managed by Barbeque.
 * Such stuff is stored in a Recipe object. Each application can use more than
 * one recipe, but a single instance must specify the one upon which base its
 * execution.
 */
class Recipe: public AttributesContainer {

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
	inline void SetPriority(uint8_t prio) {
		priority = prio;
	}

	/**
	 * @brief Insert an application working mode
	 *
	 * @param id Working mode ID
	 * @param name Working mode descripting name
	 * @param value The user QoS value of the working mode
	 */
	AwmPtr_t const AddWorkingMode(uint8_t id, std::string const & name,
					uint8_t value);

	/**
	 * @brief Return an application working mode object by specifying
	 * its identifying name
	 *
	 * @param id Working mode ID
	 * @return A shared pointer to the application working mode searched
	 */
	inline AwmPtr_t GetWorkingMode(uint8_t id) {
		if (last_awm_id == 0)
			return AwmPtr_t();
		return working_modes[id];
	}

	/**
	 * @brief All the working modes defined into the recipe
	 * @return A vector containing all the working modes
	 */
	inline AwmPtrVect_t const & WorkingModesAll() {
		working_modes.resize(last_awm_id);
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

	/** Expected AWM ID */
	uint8_t last_awm_id;

	/** The complete set of working modes descriptors defined in the recipe */
	AwmPtrVect_t working_modes;

	/** Static constraints included in the recipe */
	ConstrMap_t constraints;

	/**
	 * Store information to support the normalization of the AWM values
	 */
	struct AwmNormalInfo {
		/** Maximum value parsed */
		uint8_t max_value;
		/** Minimum value parse */
		uint8_t min_value;
		/** Diff max - min.
		 * If 0 the value will be set to 0. This is done in order to give a
		 * penalty to recipes wherein all the AWMs have been set to the same
		 * value
		 */
		uint8_t delta;
		/** Set true means that normalization has been performed yet */
		bool done;
	} norm;

	/**
	 * @brief Update the normalization info
	 *
	 * This is called when the entire set of working modes is get by an
	 * Application, during its initialization step.
	 *
	 * @param last_value The value of the last AWM inserted
	 */
	void UpdateNormalInfo(uint8_t last_value);

	/**
	 * @brief Perform the AWM values normalization
	 *
	 *                       recipe_value
	 * norm_value =  ------------------------------
	 *                         max_value
	 *
	 */
	void NormalizeAWMValues();

};

} // namespace app

} // namespace bbque

#endif // BBQUE_RECIPE_H_
