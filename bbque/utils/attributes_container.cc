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

