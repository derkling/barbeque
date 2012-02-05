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

#ifndef BBQUE_XML_RLOADER_H_
#define BBQUE_XML_RLOADER_H_

// This must be defined in order to use TinyXML++ (TiCPP) library.
// Such library is the C++ based extension of the TinyXML one (C based)
// previously used
#define TIXML_USE_TICPP

#include "bbque/plugins/recipe_loader.h"

#include <ticpp.h>
#include <tinyxml.h>

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/plugins/logger.h"
#include "bbque/plugins/plugin.h"
#include "bbque/utils/attributes_container.h"

using bbque::app::Application;
using bbque::app::AppPtr_t;
using bbque::app::AwmPtr_t;
using bbque::app::WorkingMode;
using bbque::utils::AttributesContainer;

// Parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

// Return code for internal purpose
#define __RSRC_SUCCESS 		0x0
#define __RSRC_WEAK_LOAD 	0x1
#define __RSRC_FORMAT_ERR 	0x2

// Max string length for the plugins data
#define PDATA_MAX_LEN 	20

/**
 * @brief Loader for recipes based on XML files
 *
 * The class provides methods for loading and parsing informations of
 * the applications from its recipe. This class manages recipes structured in
 * XML files.
 *
 * @note Depends on TinyXML++ (TiCPP) library (tested with version 2.5.3).
 * For library installation (on Ubuntu):
 * - Be sure TinyXML library is UNINSTALLED
 * - Checkout the project website at:
 *    http://code.google.com/p/ticpp/issues/detail?id=20
 * - Download and install the two debian packages (libticpp_<version_arch>.dev
 *    and libticpp-dev_<version<arch>.deb))
 *
 * The library should be ready to use.
 */
class XMLRecipeLoader : public RecipeLoaderIF {

public:

	/**
	 * @brief Attribute structure for plugin specific data
	 */
	typedef struct PluginAttr: public AttributesContainer::Attribute {
		/** Constructor */
		PluginAttr(std::string const & _ns, std::string const & _key):
			AttributesContainer::Attribute(_ns, _key) {}

		/** Attribute value: a string object */
		std::string str;
	} PluginAttr_t;

	/** Shared pointer to PluginAttr_t */
	typedef std::shared_ptr<PluginAttr_t> PluginAttrPtr_t;

	/**
	 * @brief Method for creating the static plugin
	 */
	static void * Create(PF_ObjectParams *);

	/**
	 * @brief Method for destroying the static plugin
	 */
	static int32_t Destroy(void *);

	/**
	 * @see RecipeLoaderIF
	 */
	ExitCode_t LoadRecipe(std::string const & recipe_name, RecipePtr_t recipe);

	/**
	 * @see RecipeLoaderIF
	 */
	std::time_t LastModifiedTime(std::string const & recipe_name);

private:

	/**
	 * @brief System logger instance
	 */
	LoggerIF *logger;

	/**
	 * Set true when the recipe loader has been configured.
	 * This is done by parsing a configuration file the first time a
	 * recipe loader is created.
	 */
	static bool configured;

	/**
	 * The directory path containing all the application recipes.
	 * Each recipe is an XML file named with suffix <tt>.recipe</tt>
	 */
	static std::string recipe_dir;

	/**
	 * Shared pointer to the recipe object
	 */
	RecipePtr_t recipe_ptr;

	/**
	 * The constructor
	 */
	XMLRecipeLoader();

	/**
	 * @brief Load RecipeLoader configuration
	 * @param params @see PF_ObjectParams
	 * @return True if the configuration has been properly loaded and object
	 * could be built, false otherwise
	 */
	static bool Configure(PF_ObjectParams * params);

	/**
	 * @brief Parse the section containing working modes data
	 * @param xml_elem The XML element from which start searching the
	 * expected section tag
	 */
	ExitCode_t loadWorkingModes(ticpp::Element * xml_elem);

	/**
	 * @brief Parse the section containing resource usages data.
	 * The method has structured for recursive calls.
	 * @param xml_elem The XML element from which start loading
	 * @param wm The working mode including this resource usages
	 * @param res_path Resource path (i.e. "arch.clusters.mem0")
	 * expected section tag
	 */
	uint8_t loadResources(ticpp::Element * xml_elem, AwmPtr_t & wm,
			std::string const & res_path);

	/**
	 * @brief Insert the resource in the working mode after checking if
	 * resource path have some matches between system resources
	 * @param wm The working mode to which add the resource usage
	 * @param res_path Resource path
	 * @param res_usage Resource usage value
	 * @return An internal error code
	 */
	uint8_t appendToWorkingMode(AwmPtr_t & wm, std::string const & res_path,
			uint64_t res_usage);

	/**
	 * @brief Parse the resource data from the xml element and add the
	 * resource usage request to the working mode
	 * @param res_elem The XML element
	 * @param wm The working mode to which add the resource usage
	 * @param res_path Resource path
	 */
	uint8_t getResourceAttributes(ticpp::Element * res_elem, AwmPtr_t & wm,
			std::string & res_path);

	/**
	 * @brief Parse the section containing plugins specific data for the
	 * application or for a working mode
	 * @param container The object (usually Application or WorkingMode to
	 * which add the plugin specific data
	 * @param xml_elem The XML element from which start searching the
	 * expected section tag
	 */
	template<class T>
	void loadPluginsData(T container, ticpp::Element * xml_elem);

	/**
	 * @brief Parse data from <plugin> element
	 *
	 * @param container The object (usually Application or WorkingMode to
	 * @param plugin_elem The XML element to parse for getting data
	 * which add the plugin specific data)
	 */
	template<class T>
	void parsePluginTag(T container, ticpp::Element * plugin_elem);

	/**
	 * @brief Parse the data nested under <plugin>
	 *
	 * @param container The object to which append the plugins data (usually
	 * an Application or a WorkingMode)
	 * @param plugdata_node The XML Node to check for data
	 * @param plugin_name The name of the plugin
	 */
	template<class T>
	void getPluginData(T container, ticpp::Node * plugdata_node,
			std::string const & plugin_name);

	/**
	 * @brief Parse the section containing constraints assertions
	 * @param xml_elem The XML element from which start searching the
	 * expected section tag
	 */
	void loadConstraints(ticpp::Element * xml_elem);

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_XML_RLOADER_H_
