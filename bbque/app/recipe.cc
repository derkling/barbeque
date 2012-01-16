/**
 *       @file  recipe.cc
 *      @brief  Implementation of class Recipe
 *
 * This implements class Recipe defined in recipe.h
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

#include "bbque/app/recipe.h"

#include <cstring>

#include "bbque/modules_factory.h"
#include "bbque/app/working_mode.h"
#include "bbque/res/resource_accounter.h"

namespace ba = bbque::app;
namespace bp = bbque::plugins;
namespace br = bbque::res;

namespace bbque { namespace app {


Recipe::Recipe(std::string const & name):
	pathname(name),
	last_awm_id(0) {

	// Get a logger
	std::string logger_name(RECIPE_NAMESPACE"." + name);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	assert(logger);

	// Clear normalization info
	memset(&norm, 0, sizeof(Recipe::AwmNormalInfo));
	norm.min_value = UINT8_MAX;
	working_modes.resize(MAX_NUM_AWM);
}

Recipe::~Recipe() {
	working_modes.clear();
	constraints.clear();
}

AwmPtr_t const Recipe::AddWorkingMode(uint8_t _id,
				std::string const & _name,
				uint8_t _value) {
	// Check if the AWMs are sequentially numbered
	if (_id != last_awm_id) {
		logger->Error("AddWorkingModes: Found ID = %d. Expected %d",
				_id, last_awm_id);
		return AwmPtr_t();
	}

	// Update info for supporting normalization
	UpdateNormalInfo(_value);

	// Insert a new working mode descriptor into the vector
	AwmPtr_t new_awm(new app::WorkingMode(_id, _name, _value));

	// Insert the AWM descriptor into the vector
	working_modes[_id] = new_awm;
	++last_awm_id;

	return working_modes[_id];
}

void Recipe::AddConstraint(std::string const & rsrc_path,
				uint64_t lb,
				uint64_t ub) {
	// Check resource existance
	br::ResourceAccounter & ra(br::ResourceAccounter::GetInstance());
	if (!ra.ExistResource(rsrc_path))
		return;

	// If there's a constraint yet, take the greater value between the bound
	// found and the one passed by argument
	ConstrMap_t::iterator cons_it(constraints.find(rsrc_path));
	if (cons_it != constraints.end()) {
		(cons_it->second)->lower = std::max((cons_it->second)->lower, lb);
		(cons_it->second)->upper = std::max((cons_it->second)->upper, ub);
		logger->Debug("Constraint (edit): %s L=%d U=%d", rsrc_path.c_str(),
						(cons_it->second)->lower,
						(cons_it->second)->upper);
		return;
	}

	// Insert a new constraint
	constraints.insert(std::pair<std::string, ConstrPtr_t>(rsrc_path,
							ConstrPtr_t(new ResourceConstraint(lb, ub))));
	logger->Debug("Constraint (new): %s L=%llu U=%llu", rsrc_path.c_str(),
					constraints[rsrc_path]->lower,
					constraints[rsrc_path]->upper);
}

void Recipe::UpdateNormalInfo(uint8_t last_value) {
	// This reset the "normalization done" flag
	norm.done = false;

	// Update the max value
	if (last_value > norm.max_value)
		norm.max_value = last_value;

	// Update the min value
	if (last_value < norm.min_value)
		norm.min_value = last_value;

	// Delta
	norm.delta = norm.max_value - norm.min_value;

	logger->Debug("AWM max value = %d", norm.max_value);
	logger->Debug("AWM min value = %d", norm.min_value);
	logger->Debug("AWM delta = %d", norm.delta);
}

void Recipe::NormalizeValues() {
	float normal_value = 0.0;

	// Return if performed yet
	if (norm.done)
		return;

	// Normalization of the whole set of AWMs
	for (int i = 0; i < last_awm_id; ++i) {
		// Normalize the value
		if (norm.delta > 0)
			// The most commmon case
			normal_value = (working_modes[i]->RecipeValue() - norm.min_value) / norm.delta;
		else if (working_modes.size() == 1)
			// There is only one AWM in the recipe
			normal_value = 1.0;
		else
			// This penalizes set of working modes having always the same QoS
			// value
			normal_value = 0.0;

		// Set the normalized value into the AWM
		working_modes[i]->SetNormalValue(normal_value);
		logger->Info("AWM %d normalized value = %.2f ",
					working_modes[i]->Id(), working_modes[i]->Value());
	}

	// Set the "normalization done" flag true
	norm.done = true;
}

} // namespace app

} // namespace bbque

