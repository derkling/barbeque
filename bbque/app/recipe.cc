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


void Recipe::AddWorkingMode(AppPtr _app, std::string const & _name,
		uint8_t _value) {

	// Allocate a new working mode descriptor
	AwmPtr new_awm(new class WorkingMode(_app, _name, _value));
	// Insert it into the structures
	working_modes.push_back(new_awm);
}


void Recipe::RemoveWorkingMode(std::string const & _name) {

	// Remove it from the vector
	std::vector<AwmPtr>::iterator awm_it = working_modes.begin();
	for ( ; awm_it != working_modes.end(); ++awm_it)
		if ((*awm_it)->Name().compare(_name) == 0)
			working_modes.erase(awm_it);
}


AwmPtr Recipe::WorkingMode(std::string const & _name) {
	AwmPtr awm_null;
	awm_null.reset();

	std::vector<AwmPtr>::const_iterator it = _GetWorkingModeIterator(_name);
	if (it != working_modes.end())
		return *it;

	// _name doesn't match any working mode
	return awm_null;
}


std::vector<AwmPtr>::const_iterator Recipe::_GetWorkingModeIterator(
		std::string	const & _name) {

	std::vector<AwmPtr>::const_iterator it = working_modes.begin();
	std::vector<AwmPtr>::const_iterator endv = working_modes.end();

	// Linear search.
	// This could seems suboptimal but we don't expect to have more
	// than 4-5 working modes per application
	for (; it < endv; ++it) {
		std::string temp = (*it)->Name();
		if (temp.compare(_name) == 0)
			break;
	}
	return it;
}

} // namespace app

} // namespace bbque

