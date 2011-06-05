/**
 *       @file  plugin_data.cc
 *      @brief  Classes for managing plugin specific data attached to
 *      Application and WorkingMode descriptors (implementation)
 *
 * This implement a class for managing plugin specific data.
 * The class provide an interface for setting and getting plugin specific
 * attributes. We expect to use this class for extending classes as
 * Application and WorkingMode.
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

#include "bbque/app/plugin_data.h"

#include <iostream>

namespace bbque { namespace app {


PluginsData::PluginsData() {
}


PluginsData::~PluginsData() {
	plugins_data.clear();
}


VoidPtr_t PluginsData::GetAttribute(std::string const & _plugin,
		std::string const & _key) {

	// Find the plugin set of pairs
	std::pair<PlugDataMap_t::iterator, PlugDataMap_t::iterator> range =
		plugins_data.equal_range(_plugin);

	// Find the attribute
	PlugDataMap_t::iterator it = range.first;
	while (it != range.second &&
		it->second.first.compare(_key) != 0) {
		++it;
	}

	// Return the value
	if (it != range.second)
		return it->second.second;
	return VoidPtr_t();
}


void PluginsData::SetAttribute(std::string const & _plugin,
		std::string const & _key, VoidPtr_t _value) {

	// Find the plugin set of pairs
	std::pair<PlugDataMap_t::iterator, PlugDataMap_t::iterator> range =
		plugins_data.equal_range(_plugin);

	// Find the attribute
	PlugDataMap_t::iterator it = range.first;
	while (it != range.second &&
		it->second.first.compare(_key) != 0) {
		++it;
	}

	// Set the attribute
	if (it != range.second)
		it->second.second = VoidPtr_t(_value);
	else
		plugins_data.insert(plugins_data.begin(),
						std::pair<std::string, DataPair_t>(_plugin,
								DataPair_t(_key, _value)));
}

} // namespace app

} // namespace bbque

