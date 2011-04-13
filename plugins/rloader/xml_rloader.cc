/**
 *       @file  xml_rloader.cc
 *      @brief  A recipe loader for recipes in XML file format
 *
 * The implements the plugin for loading recipe data from XML files.
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

#include "xml_rloader.h"

#include <cmath>
#include <iostream>
#include <boost/filesystem/operations.hpp>

#include "bbque/app/application.h"
#include "bbque/app/constraints.h"
#include "bbque/app/plugin_data.h"
#include "bbque/app/recipe.h"
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
		 (&recipe_dir)->default_value("/etc/bbque/recipes"),
		 "applications recipes folder")
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
		AppPtr_t _app, std::string const & _recipe_name, RecipePtr_t _recipe) {

	RecipeLoaderIF::ExitCode_t ret_awms;

	// The current application descriptor
	app_ptr = ba::AppPtr_t(_app);

	// The recipe object
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

		// <application> - Application root tag
		ticpp::Element * app_elem =
			root_node->FirstChildElement("application", true);

		// Load working modes data
		ret_awms = loadWorkingModes(app_elem);

		// Are there any 'static' constraints in the recipe ?
		if (ret_awms != RL_FORMAT_ERROR) {

			// Load constraints
			loadConstraints(app_elem);

			// Load plugins data
			loadPluginsData<ba::AppPtr_t>(app_ptr, app_elem);
		}

	} catch(ticpp::Exception &ex) {
		logger->Error(ex.what());
		doc.Clear();
		return RL_ABORTED;
	}
	return ret_awms;
}


std::time_t XMLRecipeLoader::LastModifiedTime(std::string const & _name) {

	boost::filesystem::path p(recipe_dir + "/" + _name + ".recipe");
	return boost::filesystem::last_write_time(p);
}


//========================[ Working modes ]===================================

RecipeLoaderIF::ExitCode_t XMLRecipeLoader::loadWorkingModes(
		ticpp::Element *_xml_elem) {

	// For each working mode we need resource usages data and (optionally)
	// plugins specific data

	uint8_t ret_res = __RSRC_SUCCESS;

	try {
		// <awms> - Application Working Modes
		ticpp::Node * awms_elem = _xml_elem->FirstChildElement("awms", true);

		// <awm>
		ticpp::Element * awm_elem = awms_elem->FirstChildElement("awm", true);
		while (awm_elem) {

			// Working mode attributes
			std::string wm_name;
			awm_elem->GetAttribute("name", &wm_name, true);
			unsigned int wm_value;
			awm_elem->GetAttribute("value", &wm_value, true);

			// Add a new working mode passing its name and value
			recipe_ptr->AddWorkingMode(app_ptr, wm_name,
					static_cast<uint8_t> (wm_value));

			// Get the descriptor of the working mode just created
			AwmPtr_t awm;
			awm = recipe_ptr->WorkingMode(wm_name);

			// Load resource usages of the working mode
			// <resources>
			ticpp::Element * resources_elem =
				awm_elem->FirstChildElement("resources", true);

			if ((ret_res = loadResources(resources_elem, awm, "")) ==
					__RSRC_FORMAT_ERR)
				// Abort loading whenever a format error occours
				return RL_FORMAT_ERROR;

			// Plugin specific data (of the AWM)
			ba::AwmPtr_t this_awm(recipe_ptr->WorkingMode(wm_name));
			if (this_awm.get() != NULL)
				loadPluginsData<ba::AwmPtr_t>(this_awm, awm_elem);

			// Next working mode
			awm_elem = awm_elem->NextSiblingElement("awm", false);
		}

	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
		return RL_ABORTED;
	}

	if (ret_res == __RSRC_WEAK_LOAD)
		return RL_WEAK_LOAD;

	return RL_SUCCESS;
}


// =======================[ Resources ]=======================================

uint8_t XMLRecipeLoader::loadResources(ticpp::Element * _xml_elem,
		AwmPtr_t & _wm,	std::string const &	_curr_path = "") {

	uint8_t ret_code = __RSRC_SUCCESS;

	try {
		// Get the resource xml element
		ticpp::Element * res_elem = _xml_elem->FirstChildElement(true);

		while (res_elem) {

			// The current resource path
			// (updated when parseResourceData returns)
			std::string res_path = _curr_path;

			// Parse the data from the resource element
			ret_code |= parseResourceData(res_elem, _wm, res_path);

			// The current resource is a container of other resources,
			// thus load the children resources recursively
			if (!res_elem->NoChildren())
				ret_code |= loadResources(res_elem, _wm, res_path);

			if (ret_code == __RSRC_FORMAT_ERR)
				return ret_code;

			// Next resource
			res_elem = res_elem->NextSiblingElement(false);
		}
	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
		return __RSRC_FORMAT_ERR;
	}

	return ret_code;
}


inline uint8_t XMLRecipeLoader::appendToWorkingMode(
		AwmPtr_t & wm, std::string const & _res_path, uint64_t _res_usage) {

	// Add the resource usage to the working mode
	ba::WorkingMode::ExitCode_t wm_err_code;
	wm_err_code = wm->AddResourceUsage(_res_path, _res_usage);

	// Check error code returned
	int int_err = static_cast<int>(wm_err_code);
	switch(int_err) {

	// Resource not found
	case (int) ba::WorkingMode::WM_RSRC_NOT_FOUND:

		// If the resource is not available in the system signal a
		// weak load, meaning that the working mode has a partial
		// definition.
		logger->Warn("'%s' recipe:\n\tResource '%s' not available.\n",
				 recipe_ptr->Path().c_str(), _res_path.c_str());
		return __RSRC_WEAK_LOAD;

	// Usage value exceeds availability
	case (int) ba::WorkingMode::WM_RSRC_USAGE_EXCEEDS:

		// If the usage value requests is greater than the total
		// availability of the resource, signal a format error.
		logger->Error("'%s' recipe:\n\tResource '%s' usage exceeds.",
					 recipe_ptr->Path().c_str(), _res_path.c_str());
		return __RSRC_FORMAT_ERR;
	}

	return __RSRC_SUCCESS;
}


inline uint8_t XMLRecipeLoader::parseResourceData(ticpp::Element * _res_elem,
		AwmPtr_t & _wm, std::string & _res_path) {

	// Resource ID
	std::string res_id;
	_res_elem->GetAttribute("id", &res_id, false);

	// Build the resource path string
	if (!_res_path.empty())
		_res_path += ".";
	_res_path += _res_elem->Value() + res_id;

	// Resource type
	std::string res_type;
	_res_elem->GetAttribute("type", &res_type, false);

	// Resource amount
	uint64_t res_usage = 0;
	_res_elem->GetAttribute("amount", &res_usage, false);

	// Units
	std::string res_units;
	_res_elem->GetAttribute("units", &res_units, false);

	// Parse the units string and update correctly the resource amount
	// value
	res_usage = ConvertValue(res_usage, res_units);

	// A true resource MUST have the "id" attribute.
	// Otherwise the xml tag is considered"resource container" (ie.
	// "arch", "clusters", ...").
	// This case is obviously skipped in loading resource usages...
	if (!res_id.empty() && (res_usage > 0))
		return appendToWorkingMode(_wm, _res_path, res_usage);

	return __RSRC_SUCCESS;
}


// =======================[ Plugins specific data ]===========================

template<class T>
void XMLRecipeLoader::loadPluginsData(T _container, ticpp::Element * _xml_elem) {

	// <plugins> [Optional]
	//  Section tag for plugin specific data.
	//  This section can be embedded in the <application> section and in the
	//  <awm> section.

	// <plugins>
	ticpp::Element * plugins_elem =
		_xml_elem->FirstChildElement("plugins",	false);

	if (plugins_elem == NULL)
		return;

	// <plugin ...>
	ticpp::Element * plug_elem = plugins_elem->FirstChildElement("plugin");

	try {
		while (plug_elem) {

			// Parse the <plugin> tag
			parsePluginTag<T>(plug_elem, _container);

			// Next plugin
			plug_elem = plug_elem->NextSiblingElement("plugin", false);
		}
	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
	}
}


template<class T>
inline void XMLRecipeLoader::parsePluginTag(ticpp::Element * _plug_elem,
		T _container) {

	try {
		// Plugin attributes
		std::string name;
		_plug_elem->GetAttribute("name", &name);

		std::string type;
		_plug_elem->GetAttribute("type", &type);

		// Is the plugin required ?
		unsigned char required = 'n';
		_plug_elem->GetAttribute("required", &required);
		bool req_flag;
		required == 'y' ? req_flag = true : req_flag = false;

		// TODO: Insert the plugin existance control

		// Create the PluginData object
		PluginDataPtr_t pdata =
			_container->AddPluginData(name, type, req_flag);

		// Plugin data nodes under <plugin>
		ticpp::Node * plugdata_node = _plug_elem->FirstChild(false);
		while (plugdata_node) {

			// Parse the data
			parsePluginData(pdata, plugdata_node);

			// Next data
			plugdata_node = plugdata_node->NextSibling(false);
		}

	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
	}
}


inline void XMLRecipeLoader::parsePluginData(
		PluginDataPtr_t & pdata, ticpp::Node * plugdata_node){

	try {
		// Is the node an element ?
		if (plugdata_node->Type() != TiXmlNode::ELEMENT)
			return;

		// Get the pair key-value
		std::string key;
		plugdata_node->GetValue(&key);
		std::string value;
		plugdata_node->ToElement()->GetText(&value, false);

		logger->Debug("Plugin %s: %s=%s", pdata->Name().c_str(),
				key.c_str(), value.c_str());

		// Insert the data
		pdata->Set(key, value);

	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
	}
}


// =======================[ Constraints ]=====================================

void XMLRecipeLoader::loadConstraints(ticpp::Element * _xml_elem) {

	// <constraints> [Optional]
	//  An application can specify bounds for resource usages
	//  over which the execution can yield an unsatisfactory
	//  behavior. This method loads static constraint assertions.
	//  Constraints may turn some working mode disabled.

	ticpp::Element * constr_elem =
		_xml_elem->NextSiblingElement("constraints", false);

	// <constraint ...>
	if (constr_elem) {

		ticpp::Element * con_elem =
		    constr_elem->FirstChildElement("constraint", false);

		try {
			// For each <constraint> element
			while (con_elem) {
				// Constraint attributes
				std::string constraint_type;
				con_elem->GetAttribute("type", &constraint_type);

				std::string resource;
				con_elem->GetAttribute("resource", &resource);

				uint32_t value;
				con_elem->GetText(&value, true);
				ba::Constraint::BoundType_t type;

				if (constraint_type.compare("L") == 0) {
					// A lower bound constraint
					type = ba::Constraint::LOWER_BOUND;
				}
				else if (constraint_type.compare("U") == 0) {
					// An upper bound constraint
					type = ba::Constraint::UPPER_BOUND;
				}
				else {
					// Uncorrected type specified
					logger->Warn("Unknown bound type");
					continue;
				}
				// Set the constraint
				app_ptr->SetConstraint(resource, type, value);

				// Next constraint tag
				con_elem = con_elem->NextSiblingElement("constraint", false);
			}
		} catch (ticpp::Exception &ex) {
			logger->Error(ex.what());
		}
	}
}

} // namespace plugins

} // namespace bque

