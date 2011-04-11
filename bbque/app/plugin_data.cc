/**
 *       @file  plugin_data.cc
 *      @brief  Classes for managing plugin specific data attached to
 *      Application and WorkingMode descriptors (implementation)
 *
 * This implement classes for managing plugin specific data. Such data are loaded
 * from the application recipe and can be attached to Application and
 * WorkingMode object.
 * Thus Barbeque plugins can retrieve and update their own data by accessing
 * objects Application and WorkingMode.
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


// =====[ PluginData ]==========================================================

PluginData::PluginData(std::string const &_plugin, std::string const &_type,
                       bool _req):
	plugin_name(_plugin),
	type(_type),
	required(_req) {
}


PluginData::ExitCode_t PluginData::Get(std::string const & _key,
		std::string & value) {

	std::map<std::string, std::string>::const_iterator it =
		str_data.find(_key);

	if (it == str_data.end())
		return PDATA_ERR_MISS_VALUE;

	value = it->second;
	return PDATA_SUCCESS;
}


PluginData::ExitCode_t PluginData::Get(std::string const & _key,
		uint32_t & value) {

	std::map<std::string, uint32_t>::const_iterator it =
		int_data.find(_key);

	if (it == int_data.end())
		return PDATA_ERR_MISS_VALUE;

	value = it->second;
	return PDATA_SUCCESS;
}


PluginData::ExitCode_t PluginData::Get(std::string const & _key, void * data) {

	std::map<std::string, void *>::const_iterator it =
		cust_data.find(_key);

	if (it == cust_data.end())
		return PDATA_ERR_MISS_VALUE;

	data = it->second;
	return PDATA_SUCCESS;
}


// =====[ PluginsDataContainer ]===============================================

PluginsDataContainer::PluginsDataContainer() {
}


PluginDataPtr_t PluginsDataContainer::AddPluginData(std::string const &_plugin,
		std::string const &_type, bool _req) {

	// Insert the plugin data into the map
	PluginDataPtr_t pdata(new PluginData(_plugin, _type, _req));
	plugins[_plugin] = pdata;
	return pdata;
}

PluginDataPtr_t PluginsDataContainer::GetPluginData(std::string const &_plugin) {
	PluginDataPtr_t pdata;
	pdata.reset();

	// Lookup the plugin data object
	std::map<std::string, PluginDataPtr_t>::const_iterator plug_it =
	    plugins.find(_plugin);

	if (plug_it != plugins.end())
		pdata = PluginDataPtr_t(plug_it->second);

	return pdata;
}

} // namespace app

} // namespace bbque

