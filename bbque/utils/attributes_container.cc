/**
 *       @file  attributes_container.cc
 *      @brief  Classes for managing specific data to attach to descriptors as
 *      Application, WorkingMode, Recipe and so on.
 *
 * This implement a class for managing specific data.
 * The class provide an interface for setting and getting specific attributes.
 * We expect to use this class to provide a support for storing attributes
 * specific of a given platform or plugin. This should be used for extending
 * information provided by descriptors as Application, WorkingMode, Recipe,
 * etc...
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  01/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/utils/attributes_container.h"
#include "bbque/utils/utility.h"

#include <iostream>


namespace bbque { namespace utils {


AttributesContainer::AttributesContainer() {
}

AttributesContainer::~AttributesContainer() {
	attributes.clear();
}

AttributesContainer::AttrPtr_t
AttributesContainer::GetAttribute(std::string const & _ns,
		std::string const & _key) {
	// Find the plugin set of pairs
	std::pair<AttributesMap_t::iterator, AttributesMap_t::iterator> range =
		attributes.equal_range(_ns);

	// Find the attribute
	AttributesMap_t::iterator it = range.first;
	while (it != range.second &&
		it->second->key.compare(_key) != 0) {
		++it;
	}

	// Return the value
	if (it != range.second) {
		return it->second;
	}
	// Null return
	return AttrPtr_t();
}

AttributesContainer::ExitCode_t
AttributesContainer::SetAttribute(AttrPtr_t _attr) {
	// Debug: check the correctness of the insertion
	DB(
		size_t before_sz = attributes.size();
	);

	// Insert into the multi-map
	attributes.insert(attributes.begin(),
			std::pair<std::string, AttrPtr_t>(_attr->ns, _attr));

	DB (
		assert(attributes.size() > before_sz);
	);

	return ATTR_OK;
}

void AttributesContainer::ClearAttribute(std::string const & _ns,
		std::string const & _key) {
	// Remove all the attributes under the namespace
	if (_key.empty()) {
		attributes.erase(_ns);
		return;
	}

	// Find the plugin set of pairs
	std::pair<AttributesMap_t::iterator, AttributesMap_t::iterator> range =
		attributes.equal_range(_ns);

	// Find the specific attribute
	AttributesMap_t::iterator it = range.first;
	while (it != range.second &&
		it->second->key.compare(_key) != 0) {
		++it;
	}

	// Remove the single attribute
	if (it == attributes.end())
		return;
	attributes.erase(it);
}

} // namespace utils

} // namespace bbque

