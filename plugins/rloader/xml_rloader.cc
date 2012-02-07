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

#include "xml_rloader.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <boost/filesystem/operations.hpp>

#include "bbque/app/application.h"
#include "bbque/app/constraints.h"
#include "bbque/app/working_mode.h"
#include "bbque/plugins/logger.h"
#include "bbque/utils/utility.h"


namespace ba = bbque::app;
namespace po = boost::program_options;

namespace bbque { namespace plugins {


/** Set true it means the plugin has read its options in the config file*/
bool XMLRecipeLoader::configured = false;

/** Recipes directory */
std::string XMLRecipeLoader::recipe_dir = "";

/** Map of options (in the Barbeque config file) for the plugin */
po::variables_map xmlrloader_opts_value;


XMLRecipeLoader::XMLRecipeLoader() {
	// Get a logger
	plugins::LoggerIF::Configuration conf("rloader.xml");
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

	if (logger)
		logger->Info("XMLRecipeLoader: plugin built");
}


bool XMLRecipeLoader::Configure(PF_ObjectParams * params) {

	if (configured)
		return true;

	// Declare the supported options
	po::options_description xmlrloader_opts_desc("XML Recipe Loader Options");
	xmlrloader_opts_desc.add_options()
		(RECIPE_LOADER_NAMESPACE"xml.recipe_dir", po::value<std::string>
		 (&recipe_dir)->default_value(BBQUE_PATH_PREFIX"/"BBQUE_PATH_RECIPES),
		 "recipes folder")
		;

	// Get configuration params
	PF_Service_ConfDataIn data_in;
	data_in.opts_desc = &xmlrloader_opts_desc;
	PF_Service_ConfDataOut data_out;
	data_out.opts_value = &xmlrloader_opts_value;

	PF_ServiceData sd;
	sd.id = RECIPE_LOADER_NAMESPACE"xml";
	sd.request = &data_in;
	sd.response = &data_out;

	int32_t response =
		params->platform_services->InvokeService(PF_SERVICE_CONF_DATA, sd);

	if (response!=PF_SERVICE_DONE)
		return false;

	fprintf(stdout, "XMLRecipeLoader: using recipe folder [%s]\n",
			recipe_dir.c_str());

	return true;
}


// =======================[ Static plugin interface ]=========================

void * XMLRecipeLoader::Create(PF_ObjectParams *params) {

	if (!Configure(params))
		return NULL;

	return new XMLRecipeLoader();
}

int32_t XMLRecipeLoader::Destroy(void *plugin) {
	if (!plugin)
		return -1;
	delete (XMLRecipeLoader *)plugin;
	return 0;
}


// =======================[ MODULE INTERFACE ]================================

RecipeLoaderIF::ExitCode_t XMLRecipeLoader::LoadRecipe(
				std::string const & _recipe_name,
				RecipePtr_t _recipe) {
	RecipeLoaderIF::ExitCode_t result;
	recipe_ptr = ba::RecipePtr_t(_recipe);

	// Plugin needs a logger
	if (!logger) {
		std::cout << "Error: Plugin 'XMLRecipeLoader' needs a logger."
				  << std::endl;
		return RL_ABORTED;
	}

	ticpp::Document doc;
	try {
		// Load the recipe parsing an XML file
		std::string path(recipe_dir + "/" + _recipe_name + ".recipe");
		doc.LoadFile(path.c_str());

		// <BarbequeRTRM> - Recipe root tag
		ticpp::Node * root_node = doc.FirstChild();
		root_node = root_node->NextSibling("BarbequeRTRM", true);

		ticpp::Element * app_elem =
				root_node->FirstChildElement("application", true);
		uint16_t prio = 0;
		app_elem->GetAttribute("priority", &prio, false);
		recipe_ptr->SetPriority(prio);

		// Application Working Modes
		result = loadWorkingModes(app_elem);

		// "Static" constraints and plugins specific data
		if (result != RL_FORMAT_ERROR) {
			loadConstraints(app_elem);
			loadPluginsData<ba::RecipePtr_t>(recipe_ptr, app_elem);
		}

	} catch(ticpp::Exception &ex) {
		logger->Error(ex.what());
		doc.Clear();
		return RL_ABORTED;
	}
	return result;
}


std::time_t XMLRecipeLoader::LastModifiedTime(std::string const & _name) {
	boost::filesystem::path p(recipe_dir + "/" + _name + ".recipe");
	return boost::filesystem::last_write_time(p);
}


//========================[ Working modes ]===================================

RecipeLoaderIF::ExitCode_t XMLRecipeLoader::loadWorkingModes(
		ticpp::Element *_xml_elem) {
	// Loading resources info exit code
	uint8_t result = __RSRC_SUCCESS;

	try {
		// For each working mode we need resource usages data and (optionally)
		// plugins specific data
		ticpp::Node * awms_elem = _xml_elem->FirstChildElement("awms", true);
		ticpp::Element * awm_elem = awms_elem->FirstChildElement("awm", true);
		while (awm_elem) {
			// Working mode attributes
			unsigned int wm_id;
			awm_elem->GetAttribute("id", &wm_id, true);
			std::string wm_name;
			awm_elem->GetAttribute("name", &wm_name, false);
			unsigned int wm_value;
			awm_elem->GetAttribute("value", &wm_value, true);

			// The awm ID must be unique!
			if (recipe_ptr->GetWorkingMode(wm_id)) {
				logger->Error("AWM ""%s"" error: Double ID found %d",
								wm_name.c_str(), wm_id);
				return RL_FORMAT_ERROR;
			}

			// Add a new working mode passing its name and value
			// Note: IDs must be numbered from 0 to N
			AwmPtr_t awm(recipe_ptr->AddWorkingMode(wm_id, wm_name,
							static_cast<uint8_t> (wm_value)));
			if (!awm) {
				logger->Error("AWM ""%s"" error: Wrong ID specified %d",
								wm_name.c_str(), wm_id);
				return RL_FORMAT_ERROR;
			}

			// Load resource usages of the working mode
			ticpp::Element * resources_elem =
				awm_elem->FirstChildElement("resources", true);
			if ((result = loadResources(resources_elem, awm, "")) ==
					__RSRC_FORMAT_ERR)
				return RL_FORMAT_ERROR;

			// AWM plugin specific data
			loadPluginsData<ba::AwmPtr_t>(awm, awm_elem);

			// Next working mode
			awm_elem = awm_elem->NextSiblingElement("awm", false);
		}

	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
		return RL_ABORTED;
	}

	if (result == __RSRC_WEAK_LOAD)
		return RL_WEAK_LOAD;

	return RL_SUCCESS;
}


// =======================[ Resources ]=======================================

uint8_t XMLRecipeLoader::loadResources(ticpp::Element * _xml_elem,
		AwmPtr_t & _wm,
		std::string const & _curr_path = "") {
	// Exit code
	uint8_t result = __RSRC_SUCCESS;

	try {
		// Get the resource xml element
		ticpp::Element * res_elem = _xml_elem->FirstChildElement(true);
		while (res_elem) {

			// Parse the attributes from the resource element
			std::string res_path = _curr_path;
			result |= getResourceAttributes(res_elem, _wm, res_path);
			if (result >= __RSRC_FORMAT_ERR)
				return result;

			// The current resource is a container of other resources,
			// thus load the children resources recursively
			if (!res_elem->NoChildren()) {
				result |= loadResources(res_elem, _wm, res_path);
				if (result >= __RSRC_FORMAT_ERR)
					return result;
			}

			// Next resource
			res_elem = res_elem->NextSiblingElement(false);
		}
	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
		return __RSRC_FORMAT_ERR;
	}

	return result;
}


inline uint8_t XMLRecipeLoader::appendToWorkingMode(AwmPtr_t & wm,
				std::string const & _res_path,
				uint64_t _res_usage) {
	// Add the resource usage to the working mode
	ba::WorkingMode::ExitCode_t result;
	result = wm->AddResourceUsage(_res_path, _res_usage);

	// Check error code returned
	int int_err = static_cast<int>(result);
	switch(int_err) {
	// Resource not found :
	// Signal a weak load (some resources are missing)
	case (int) ba::WorkingMode::WM_RSRC_NOT_FOUND:
		logger->Warn("'%s' recipe:\n\tResource '%s' not available.\n",
				 recipe_ptr->Path().c_str(), _res_path.c_str());
		return __RSRC_WEAK_LOAD;
	// Usage value exceeds availability:
	// The working mode cannot be accepted.
	case (int) ba::WorkingMode::WM_RSRC_USAGE_EXCEEDS:
		logger->Error("'%s' recipe:\n\tResource '%s' usage exceeds.",
					 recipe_ptr->Path().c_str(), _res_path.c_str());
		return __RSRC_FORMAT_ERR;
	}

	return __RSRC_SUCCESS;
}


inline uint8_t XMLRecipeLoader::getResourceAttributes(
				ticpp::Element * _res_elem,
				AwmPtr_t & _wm,
				std::string & _res_path) {
	// Resource ID
	std::string res_id;
	_res_elem->GetAttribute("id", &res_id, false);

	// Build the resource path string
	if (!_res_path.empty())
		_res_path += ".";
	_res_path += _res_elem->Value() + res_id;

	// Resource quantity request and units
	uint64_t res_usage = 0;
	_res_elem->GetAttribute("qty", &res_usage, false);
	std::string res_units;
	_res_elem->GetAttribute("units", &res_units, false);

	// The usage requested must be > 0
	if (!_res_elem->GetAttribute("qty").empty() && res_usage <= 0) {
		logger->Error("Resource ""%s"": usage value not valid (%llu)",
				_res_path.c_str(), res_usage);
		return __RSRC_FORMAT_ERR;
	}

	// If the quantity is 0 return without adding the resource request to the
	// current AWM
	if (res_usage == 0)
		return __RSRC_SUCCESS;

	// Convert the usage value accordingly to the units, and then append the
	// request to the working mode.
	res_usage = ConvertValue(res_usage, res_units);
	return appendToWorkingMode(_wm, _res_path, res_usage);
}


// =======================[ Plugins specific data ]===========================

template<class T>
void XMLRecipeLoader::loadPluginsData(T _container,
		ticpp::Element * _xml_elem) {
	// <plugins> [Optional]
	// Section tag for plugin specific data. This can be included into the
	// <application> section and into the <awm> section.
	ticpp::Element * plugins_elem =
		_xml_elem->FirstChildElement("plugins",	false);
	if (plugins_elem == NULL)
		return;

	try {
		// Parse the <plugin> tags
		ticpp::Element * plug_elem =
				plugins_elem->FirstChildElement("plugin", false);

		while (plug_elem) {
			parsePluginTag<T>(_container, plug_elem);
			plug_elem = plug_elem->NextSiblingElement("plugin", false);
		}
	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
	}
}


template<class T>
inline void XMLRecipeLoader::parsePluginTag(T _container,
		ticpp::Element * _plug_elem) {
	try {
		// Plugin attributes
		std::string name;
		_plug_elem->GetAttribute("name", &name);

		// Plugin data in <plugin>
		ticpp::Node * plugdata_node = _plug_elem->FirstChild(false);
		while (plugdata_node) {
			getPluginData<T>(_container, plugdata_node, name);
			plugdata_node = plugdata_node->NextSibling(false);
		}

	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
	}
}


template<class T>
inline void XMLRecipeLoader::getPluginData(T _container,
		ticpp::Node * plugdata_node,
		std::string const & _plug_name) {
	try {
		// Is the node an element ?
		if (plugdata_node->Type() != TiXmlNode::ELEMENT)
			return;

		// Get the pair key - value
		std::string key, value;
		plugdata_node->GetValue(&key);
		plugdata_node->ToElement()->GetText(&value, false);

		// Set the plugin data
		PluginAttrPtr_t pattr(new PluginAttr_t(_plug_name, key));
		pattr->str = value;
		_container->SetAttribute(pattr);

	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
	}
}


// =======================[ Constraints ]=====================================

void XMLRecipeLoader::loadConstraints(ticpp::Element * _xml_elem) {

	// <constraints> [Optional]
	// An application can specify bounds for resource usages
	// over which the execution can yield an unsatisfactory
	// behavior.
	// This method loads static constraint assertions.
	// Constraints may disable some working mode.
	ticpp::Element * constr_elem =
		_xml_elem->FirstChildElement("constraints", false);
	if (!constr_elem)
		return;

	try {
		// Constraint tags
		ticpp::Element * con_elem =
			constr_elem->FirstChildElement("constraint", false);
		while (con_elem) {
			// Attributes
			std::string constraint_type;
			con_elem->GetAttribute("type", &constraint_type);
			std::string resource;
			con_elem->GetAttribute("resource", &resource);
			uint32_t value;
			con_elem->GetAttribute("bound", &value);

			// Add the constraint
			if (constraint_type.compare("L") == 0) {
				recipe_ptr->AddConstraint(resource, value, 0);
			}
			else if (constraint_type.compare("U") == 0) {
				recipe_ptr->AddConstraint(resource, 0, value);
			}
			else {
				logger->Warn("Constraint: unknown bound type");
				continue;
			}

			// Next constraint
			con_elem = con_elem->NextSiblingElement("constraint", false);
		}
	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
	}
}

} // namespace plugins

} // namespace bque

