/**
 *       @file  xml_rloader.h
 *      @brief Recipe loader for XML file formats
 *
* This defines the plugin for making recipe loading from an XML file
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

#ifndef BBQUE_XML_RLOADER_H_
#define BBQUE_XML_RLOADER_H_

// This must be defined in order to use TinyXML++ (TiCPP) library.
// Such library is the C++ based extension of the TinyXML one (C based)
// previously used
#define TIXML_USE_TICPP

#include "bbque/plugins/recipe_loader.h"

#include <ticpp.h>
#include <tinyxml.h>

#include "bbque/app/plugin_data.h"
#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/plugins/logger.h"
#include "bbque/plugins/plugin.h"

using bbque::app::Application;
using bbque::app::Recipe;
using bbque::app::WorkingMode;
using bbque::app::PluginData;

// Parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

// Return code for internal purpose
#define __RSRC_SUCCESS 		0x0
#define __RSRC_WEAK_LOAD 	0x1
#define __RSRC_FORMAT_ERR 	0x2

/**
 * @class XMLRecipeLoader
 *
 * @brief Loader for recipes based on XML files
 *
 * The class provides methods for loading and parsing informations of
 * the applications from its recipe. This class manages recipes structured in
 * XML files.
 *
 * @note Depends on TinyXML++ (TiCPP) library (tested with version 2.5.3).
 * For library installation (on Ubuntu):<br>
 * <ul>
 * <li> Be sure TinyXML library is UNINSTALLED
 * <li> Checkout the project website at:
 * 		http://code.google.com/p/ticpp/issues/detail?id=20
 *
 * <li>Download and install the two debian packages (libticpp_<version_arch>.dev
 *   and libticpp-dev_<version<arch>.deb))
 *
 * <br>The library should be ready to use.
 */
class XMLRecipeLoader : public RecipeLoaderIF {

public:

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
	ExitCode_t LoadRecipe(std::shared_ptr<Application> app,
			std::string const & recipe_path,
			std::shared_ptr<Recipe> recipe);

	/**
	 * @see RecipeLoaderIF
	 */
	std::time_t LastModifiedTime(std::string const & path);

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
	std::shared_ptr<Recipe> recipe_ptr;

	/**
	 * Shared pointer to the application requiring the recipe
	 */
	std::shared_ptr<Application> app_ptr;

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
	uint8_t loadResources(ticpp::Element * xml_elem,
			std::shared_ptr<WorkingMode> & wm,
			std::string const & res_path);

	/**
	 * @brief Insert the resource in the working mode after checking if
	 * resource path have some matches between system resources
	 * @param wm The working mode to which add the resource usage
	 * @param res_path Resource path
	 * @param res_usage Resource usage value
	 * @return An internal error code
	 */
	uint8_t appendToWorkingMode(std::shared_ptr<WorkingMode> & wm,
		std::string const & res_path, ulong res_usage);

	/**
	 * @brief Parse the resource data from the xml element and add the
	 * resource usage request to the working mode
	 * @param res_elem The XML element
	 * @param wm The working mode to which add the resource usage
	 * @param res_path Resource path
	 */
	uint8_t parseResourceData(ticpp::Element * res_elem,
		std::shared_ptr<WorkingMode> & wm, std::string & res_path);

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
	 * @param plug_elem The XML element to parse for getting data
	 * @param container The object (usually Application or WorkingMode to
	 * which add the plugin specific data)
	 */
	template<class T>
	void parsePluginTag(ticpp::Element * plug_elem, T container);

	/**
	 * @brief Parse the data nested under <plugin>
	 * @param pdata PluginData object to fill
	 * @param plugdata_node The XML Node to check for data
	 */
	void parsePluginData(std::shared_ptr<PluginData> & pdata,
		ticpp::Node * plugdata_node);

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

