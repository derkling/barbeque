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
#include "bbque/app/working_mode.h"

namespace bbque { namespace app {


Recipe::~Recipe() {
	working_modes.clear();
}


AwmPtr_t & Recipe::AddWorkingMode(AppPtr_t _app,
				uint16_t _id,
				std::string const & _name,
				uint16_t _value) {
	// Insert a new working mode descriptor into the map
	AwmPtr_t new_awm(new class WorkingMode(_app, _id, _name, _value));
	working_modes.insert(std::pair<uint16_t, AwmPtr_t>(_id, new_awm));
	return working_modes[_id];
}


AwmPtr_t Recipe::WorkingMode(uint16_t _id) {
	AwmMap_t::iterator it = working_modes.find(_id);
	if (it == working_modes.end())
		return AwmPtr_t();
	return it->second;
}

} // namespace app

} // namespace bbque

