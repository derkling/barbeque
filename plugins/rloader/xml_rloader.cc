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
#include <cassert>
#include <iostream>
#include <boost/filesystem/operations.hpp>

#include "bbque/platform_proxy.h"
#include "bbque/app/application.h"
#include "bbque/app/working_mode.h"
#include "bbque/res/resource_constraints.h"
#include "bbque/plugins/logger.h"
#include "bbque/utils/utility.h"

namespace ba = bbque::app;
namespace br = bbque::res;
namespace po = boost::program_options;

using ba::WorkingModeStatusIF;
using br::ResourcePathUtils;
using br::ConvertValue;

namespace bbque { namespace plugins {


/** Set true it means the plugin has read its options in the config file*/
bool XMLRecipeLoader::configured = false;

/** Recipes directory */
std::string XMLRecipeLoader::recipe_dir = "";

/** Map of options (in the Barbeque config file) for the plugin */
po::variables_map xmlrloader_opts_value;


XMLRecipeLoader::XMLRecipeLoader() {
	// Get a logger
	plugins::LoggerIF::Configuration conf(MODULE_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		if (daemonized)
			syslog(LOG_INFO, "Build XML RecipeLoader plugin [%p] FAILED "
					"(Error: missing logger module)", (void*)this);
		else
			fprintf(stdout, FI("Build XML RecipeLoader plugin [%p] FAILED "
					"(Error: missing logger module)\n"), (void*)this);
	}

	assert(logger);
	logger->Debug("Built XML RecipeLoader object @%p", (void*)this);
}


bool XMLRecipeLoader::Configure(PF_ObjectParams * params) {

	if (configured)
		return true;

	// Declare the supported options
	po::options_description xmlrloader_opts_desc("XML Recipe Loader Options");
	xmlrloader_opts_desc.add_options()
		(MODULE_CONFIG".recipe_dir", po::value<std::string>
		 (&recipe_dir)->default_value(BBQUE_PATH_PREFIX "/" BBQUE_PATH_RECIPES),
		 "recipes folder")
		;

	// Get configuration params
	PF_Service_ConfDataIn data_in;
	data_in.opts_desc = &xmlrloader_opts_desc;
	PF_Service_ConfDataOut data_out;
	data_out.opts_value = &xmlrloader_opts_value;

	PF_ServiceData sd;
	sd.id = MODULE_NAMESPACE;
	sd.request = &data_in;
	sd.response = &data_out;

	int32_t response =
		params->platform_services->InvokeService(PF_SERVICE_CONF_DATA, sd);

	if (response!=PF_SERVICE_DONE)
		return false;

	if (daemonized)
		syslog(LOG_INFO, "Using XMLRecipeLoader recipe folder [%s]",
				recipe_dir.c_str());
	else
		fprintf(stdout, FI("Using XMLRecipeLoader recipe folder [%s]\n"),
				recipe_dir.c_str());

	return true;
}


// =======================[ Static plugin interface ]=========================

void * XMLRecipeLoader::Create(PF_ObjectParams *params) {

	if (!Configure(params))
		return nullptr;

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
	ticpp::Document doc;
	ticpp::Node    * root_node = nullptr;
	ticpp::Element * app_elem  = nullptr;
	ticpp::Element * bbq_elem  = nullptr;
	ticpp::Element * pp_elem   = nullptr;
	uint16_t prio = 0;
	int maj, min;
	std::string version_id;


	// Recipe object
	recipe_ptr = _recipe;

	// Plugin needs a logger
	if (!logger) {
		fprintf(stderr, FE("Error: Plugin 'XMLRecipeLoader' needs a logger\n"));
		result = RL_ABORTED;
		goto error;
	}

	try {
		// Load the recipe parsing an XML file
		std::string path(recipe_dir + "/" + _recipe_name + ".recipe");
		doc.LoadFile(path.c_str());

		// <BarbequeRTRM> - Recipe root tag
		root_node = doc.FirstChild();
		root_node = root_node->NextSibling("BarbequeRTRM", true);

		// Recipe version control
		bbq_elem = root_node->ToElement();
		bbq_elem->GetAttribute("recipe_version", &version_id);
		logger->Debug("Recipe version = %s", version_id.c_str());
		sscanf(version_id.c_str(), "%d.%d", &maj, &min);
		if (maj < RECIPE_MAJOR_VERSION ||
				(maj >= RECIPE_MAJOR_VERSION && min < RECIPE_MINOR_VERSION)) {
			logger->Error("Recipe version mismatch (REQUIRED %d.%d). "
					"Found %d.%d", RECIPE_MAJOR_VERSION, RECIPE_MINOR_VERSION,
					maj, min);
			result = RL_VERSION_MISMATCH;
			goto error;
		}

		// <application>
		app_elem = root_node->FirstChildElement("application", true);
		app_elem->GetAttribute("priority", &prio, false);
		recipe_ptr->SetPriority(prio);

		// Load the proper platform section
		pp_elem = LoadPlatform(app_elem);
		if (!pp_elem) {
			result = RL_PLATFORM_MISMATCH;
			goto error;
		}

		// Application Working Modes
		result = LoadWorkingModes(pp_elem);
		if (result != RL_SUCCESS)
			goto error;

		// "Static" constraints and plugins specific data
		LoadConstraints(pp_elem);
		LoadPluginsData<ba::RecipePtr_t>(recipe_ptr, pp_elem);

	} catch(ticpp::Exception &ex) {
		logger->Error(ex.what());
		result = RL_ABORTED;
		goto error;
	}

	// Regular exit
	return result;

error:
	doc.Clear();
	recipe_ptr = ba::RecipePtr_t();
	return result;
}


ticpp::Element * XMLRecipeLoader::LoadPlatform(ticpp::Element * _xml_elem) {
	ticpp::Element * pp_elem     = nullptr;
	ticpp::Element * pp_gen_elem = nullptr;

#ifndef CONFIG_BBQUE_TEST_PLATFORM_DATA
	const char * sys_platform_id;
	std::string platform_id;
	PlatformProxy & pp(PlatformProxy::GetInstance());
	bool platform_matched = false;
#endif

	try {
		// <platform>
		pp_elem = _xml_elem->FirstChildElement("platform", true);
#ifndef CONFIG_BBQUE_TEST_PLATFORM_DATA
		// System platform
		sys_platform_id = pp.GetPlatformID();
		if (!sys_platform_id) {
			logger->Error("Unable to get the system platform ID");
			assert(sys_platform_id != nullptr);
			return nullptr;
		}

		// Look for the platform section matching the system platform id
		while (pp_elem && !platform_matched) {
			pp_elem->GetAttribute("id", &platform_id, true);
			if (platform_id.compare(sys_platform_id) == 0) {
				logger->Info("Platform required: '%s' matching OK",
						platform_id.c_str());
				platform_matched = true;
				break;
			}

			// Keep track of the "generic" platform section (if any)
			if (!pp_gen_elem
					&& (platform_id.compare(PLATFORM_ID_GENERIC) ==	0)) {
				pp_gen_elem = pp_elem;
			}

			// Next platform section
			pp_elem = pp_elem->NextSiblingElement("platform", false);
		}

		// Does the required platform match the system platform?
		if (!platform_matched) {
			logger->Error("Platform mismatch: cannot find (system) ID '%s'",
					sys_platform_id);

			// Generic section found?
			if (pp_gen_elem) {
				logger->Warn("Platform mismatch: section '%s' will be parsed",
						PLATFORM_ID_GENERIC);
				return pp_gen_elem;
			}
		}
#else
		logger->Warn("TPD enabled: no platform ID check performed");
#endif

	} catch(ticpp::Exception &ex) {
		logger->Error(ex.what());
		return nullptr;
	}

	return pp_elem;
}


std::time_t XMLRecipeLoader::LastModifiedTime(std::string const & _name) {
	boost::filesystem::path p(recipe_dir + "/" + _name + ".recipe");
	return boost::filesystem::last_write_time(p);
}


//========================[ Working modes ]===================================

RecipeLoaderIF::ExitCode_t XMLRecipeLoader::LoadWorkingModes(
		ticpp::Element *_xml_elem) {
	uint8_t result = __RSRC_SUCCESS;
	unsigned int wm_id;
	unsigned int wm_value;
	std::string wm_name;
	ticpp::Node    * awms_elem      = nullptr;
	ticpp::Element * resources_elem = nullptr;
	ticpp::Element * awm_elem       = nullptr;

	try {
		// For each working mode we need resource usages data and (optionally)
		// plugins specific data
		awms_elem = _xml_elem->FirstChildElement("awms", true);
		awm_elem = awms_elem->FirstChildElement("awm", true);
		while (awm_elem) {
			// Working mode attributes
			awm_elem->GetAttribute("id", &wm_id, true);
			awm_elem->GetAttribute("name", &wm_name, false);
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
			resources_elem = awm_elem->FirstChildElement("resources", true);
			if ((result = LoadResources(resources_elem, awm, "")) ==
					__RSRC_FORMAT_ERR)
				return RL_FORMAT_ERROR;

			// AWM plugin specific data
			LoadPluginsData<ba::AwmPtr_t>(awm, awm_elem);

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

uint8_t XMLRecipeLoader::LoadResources(ticpp::Element * _xml_elem,
		AwmPtr_t & _wm,
		std::string const & _curr_path = "") {
	uint8_t result = __RSRC_SUCCESS;
	std::string res_path;
	ticpp::Element * res_elem = nullptr;

	try {
		// Get the resource xml element
		res_elem = _xml_elem->FirstChildElement(true);
		while (res_elem) {

			// Parse the attributes from the resource element
			res_path = _curr_path;
			result |= GetResourceAttributes(res_elem, _wm, res_path);
			if (result >= __RSRC_FORMAT_ERR)
				return result;

			// The current resource is a container of other resources,
			// thus load the children resources recursively
			if (!res_elem->NoChildren()) {
				result |= LoadResources(res_elem, _wm, res_path);
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


uint8_t XMLRecipeLoader::AppendToWorkingMode(AwmPtr_t & wm,
		std::string const & _res_path,
		uint64_t _res_usage) {
	WorkingModeStatusIF::ExitCode_t result;

	// Add the resource usage to the working mode
	result = wm->AddResourceUsage(_res_path, _res_usage);

	// Resource not found: Signal a weak load (some resources are missing)
	if (result == WorkingModeStatusIF::WM_RSRC_NOT_FOUND) {
		logger->Warn("'%s' recipe:\n\tResource '%s' not available.\n",
				recipe_ptr->Path().c_str(), _res_path.c_str());
		return __RSRC_WEAK_LOAD;
	}

	return __RSRC_SUCCESS;
}

uint8_t XMLRecipeLoader::GetResourceAttributes(
		ticpp::Element * _res_elem,
		AwmPtr_t & _wm,
		std::string & _res_path) {
	uint64_t res_usage = 0;
	std::string res_units;
	std::string res_id;

	// Resource ID
	_res_elem->GetAttribute("id", &res_id, false);

	// Build the resource path string
	if (!_res_path.empty())
		_res_path += ".";
	_res_path += _res_elem->Value() + res_id;

	// Resource quantity request and units
	_res_elem->GetAttribute("qty", &res_usage, false);
	_res_elem->GetAttribute("units", &res_units, false);

	// The usage requested must be > 0
	if (!_res_elem->GetAttribute("qty").empty() && res_usage <= 0) {
		logger->Error("Resource ""%s"": usage value not valid (%" PRIu64 ")",
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
	return AppendToWorkingMode(_wm, _res_path, res_usage);
}


// =======================[ Plugins specific data ]===========================

template<class T>
void XMLRecipeLoader::LoadPluginsData(T _container,
		ticpp::Element * _xml_elem) {
	ticpp::Element * plugins_elem = nullptr;
	ticpp::Element * plug_elem    = nullptr;

	// <plugins> [Optional]
	// Section tag for plugin specific data. This can be included into the
	// <application> section and into the <awm> section.
	plugins_elem = _xml_elem->FirstChildElement("plugins",	false);
	if (plugins_elem == nullptr)
		return;

	try {
		// Parse the <plugin> tags
		plug_elem = plugins_elem->FirstChildElement("plugin", false);

		while (plug_elem) {
			ParsePluginTag<T>(_container, plug_elem);
			plug_elem = plug_elem->NextSiblingElement("plugin", false);
		}
	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
	}
}


template<class T>
void XMLRecipeLoader::ParsePluginTag(T _container,
		ticpp::Element * _plug_elem) {
	std::string name;
	ticpp::Node * plugdata_node = nullptr;

	try {
		// Plugin attributes
		_plug_elem->GetAttribute("name", &name);

		// Plugin data in <plugin>
		plugdata_node = _plug_elem->FirstChild(false);
		while (plugdata_node) {
			GetPluginData<T>(_container, plugdata_node, name);
			plugdata_node = plugdata_node->NextSibling(false);
		}

	} catch (ticpp::Exception &ex) {
		logger->Error(ex.what());
	}
}


template<class T>
void XMLRecipeLoader::GetPluginData(T _container,
		ticpp::Node * plugdata_node,
		std::string const & _plug_name) {
	std::string key;
	std::string value;
	try {
		// Is the node an element ?
		if (plugdata_node->Type() != TiXmlNode::ELEMENT)
			return;

		// Get the pair key - value
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

void XMLRecipeLoader::LoadConstraints(ticpp::Element * _xml_elem) {
	ticpp::Element * constr_elem = nullptr;
	ticpp::Element * con_elem    = nullptr;
	std::string resource;
	std::string constraint_type;
	uint32_t value;

	// <constraints> [Optional]
	// An application can specify bounds for resource usages
	// over which the execution can yield an unsatisfactory
	// behavior.
	// This method loads static constraint assertions.
	// Constraints may disable some working mode.
	constr_elem = _xml_elem->FirstChildElement("constraints", false);
	if (!constr_elem)
		return;

	try {
		// Constraint tags
		con_elem = constr_elem->FirstChildElement("constraint", false);
		while (con_elem) {
			// Attributes
			con_elem->GetAttribute("type", &constraint_type);
			con_elem->GetAttribute("resource", &resource);
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

