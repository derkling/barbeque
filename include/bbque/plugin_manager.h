/**
 *       @file  plugin_manager.h
 *      @brief  The Barbeque RTRM Plugin Manager
 *
 * This provides the definition of the Puligin Manager for the Barbeque RTRM.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  01/13/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */


#ifndef BBQUE_PLUGIN_MANAGER_H_
#define BBQUE_PLUGIN_MANAGER_H_

#include <cstdint>
#include <memory>
#include <vector>
#include <map>

#include "bbque/object.h"
#include "bbque/plugins/plugin.h"

namespace bbque { namespace plugins {

class DynamicLibrary;
struct ObjectAdapterIF;

/**
 * @class PluginManager
 * @brief This module provides a convenient interface for loading plugin based
 * extensions and look up for them at run-time.
 * This plugin framework is based on the reference implementation described
 * in: "Building Your Own Plugin Framework", by Gigi Sayfan.
 * A complete description could be found on-line at this address:<br>
 * http://www.drdobbs.com/cpp/204202899
 */
class PluginManager {

	/**
	 * Maps dynamic libraries paths to their representing objects.
	 */
	typedef std::map<std::string, std::shared_ptr<DynamicLibrary>> DynamicLibraryMap;

	/**
	 * A vector of plugins exit function reference.
	 */
	typedef std::vector<PF_ExitFunc> ExitFuncVec;

	/**
	 * A vector of modules registration parameters.
	 */
	typedef std::vector<PF_RegisterParams> RegistrationVec;

public:

	/**
	 * Maps module names (either static or dynamic) to their registration
	 * parameters.
	 */
	typedef std::map<std::string, PF_RegisterParams> RegistrationMap;

	/**
	 * @brief   Return a reference to the (singleton) plugin manager.
	 */
	static PluginManager & GetInstance();

	/**
	 * @brief   Initialize a plugin using the specified entry point
	 * The application may "restart" static plugins by calling this method
	 * for each one.
	 * @param   initFunc The plugin entry point.
	 * @return  0 on success, -1 on initialization failure.
	 */
	static int32_t InitializePlugin(PF_InitFunc initFunc);

	/**
	 * @brief   Load all plugins found within the specified directory.
	 * Dynamic plugins are usually all be deployed in a dedicated directory
	 * (by default /var/lib/bbque/plugins). The application should invoke this
	 * method and pass the dedicated directory path.  The PluginManager scans
	 * all the files in this directory and load every dynamic library (with
	 * filename ending by ".so").<br>
	 * The application may alternatively call the load() method, which loads a
	 * single plugin if it wants fine-grained control about what plugins are
	 * loaded exactly.
	 * @param   pluginDirectory the directory where to look for plugins
	 * @param   func a pointer to the invoke method to access platform services.
	 * @return  0 on success, a negative value on error (-1: empty directory,
	 * -2: directory not existing)
	 */
	int32_t LoadAll(const std::string & pluginDirectory,
					PF_InvokeServiceFunc func = NULL);

	/**
	 * @brief   Load the specified plugin
	 * Alternatively to the LoadAll, the application could loads a single
	 * plugin thus allowing a fine-grained control about what plugins are
	 * loaded exactly.
	 * @param  The path of the plugin to load
	 * @return  0 on success, a negative value on error.
	 */
	int32_t LoadByPath(const std::string & path);

	/**
	 * @brief   Create an object instance of the specified type.
	 * When the application needs to create a new plugin object (either
	 * dynamic or static) it can call this method and
	 * passes an object type and an adaptor. The PluginManager creates the
	 * object using the creation function the plugins has specified, at
	 * registration time, and eventually adapts it from C to C++ using the
	 * specified object adapter.<br>
	 * @note This method look for the nearest matching plugin object which
	 * name starts as the required "id", e.g. a request for a "logger." object
	 * will match either "logger.console" and "logger.log4cpp"; when both
	 * objects are registered the first matching in alphabetical order is
	 * built end returned.
	 * @param   id the string name of the required object
	 * @data    the initialization data defined by the plugin interface
	 * @return  a reference to the required object, NULL on error.
	 */
	void * CreateObject(const std::string & id,
				 void * data = NULL, ObjectAdapterIF * adapter = NULL);

	/**
	 * @brief   Release all plugins management resources.
	 * This method should be called by the application after it destroyed all
	 * the plugin objects it created. It calls the exit function of all the
	 * plugins (both static and dynamic), unloads all the dynamic plugins and
	 * clears all the internal data structures.
	 * @note    this is automatically called by the class destructor.
	 * @return  0 on success, a negative value on errors.
	 */
	int32_t Shutdown();

	/**
	 * @brief   Register a new plugins object with the specifed name and
	 * parameters
	 * This is a PluginManager service through which the plugin registers its
	 * objects (without it there will be no plugin system).  This function
	 * accepts a string that <i>uniquely identifies</i> the object type or a
	 * wildcard "*", if the name has already been used by another plugin the
	 * registration fails.
	 * @note    in order to avoid collisions, plugins object names should be
	 * defined using a proper namespace syntax. Each plugin object should
	 * implement a certain object interface and thus are expected to have a
	 * name within this interface namespace. In instance, a Log4Cpp object
	 * implementing the LoggerIF interface, should be names "logger.log4cpp".
	 * This naming allow to match the name and build such an object by
	 * requiring the plugin manager either a (generic) "logger." object, or a
	 * more specific "logger.log4cpp" object.
	 * @param   id the object name identifier
	 * @param   params the plugin object registration parames
	 * @return  0 on success, a negative number on errors (-1: invalid params,
	 * -2: version mismatching, -3 name already used);
	 */
	static int32_t RegisterObject(const char * id,
					const PF_RegisterParams * params);

	/**
	 * @brief   Get the code language (C or C++) of the specified module
	 * @param   id the module name identifier to query
	 * @return  the module code language
	 */
	PF_ProgrammingLanguage GetModuleLanguage(const std::string & id);


	/**
	 * @brief   Get a reference to the map of registerd objects
	 * @return  a reference to the map of registerd objects
	 */
	const RegistrationMap & GetRegistrationMap();

	/**
	 * @brief   Get a reference to the platform services
	 * @return  a reference to the platform services
	 */
	PF_PlatformServices & GetPlatformServices();

private:

	/**
	 * @brief   Build a new plugin manager
	 */
	PluginManager();

	/**
	 * @brief   Release the current plugin manager instance
	 */
	~PluginManager();

	/**
	 * @brief   Clone a plugin manager
	 * @param   A reference to a plugin manager
	 */
	PluginManager(const PluginManager &);

	/**
	 * @brief   Load the specified shared library
	 * @param   path the filesystem path of the library (.so) to load
	 * @param   errorString a reference to a string to host an eventually error message
	 * @return  a reference to an object representing a loaded shared library
	 */
	DynamicLibrary * LoadLibrary(const std::string & path, std::string & errorString);

private:

	/**
	 * Set true while a plugin is initializing
	 */
	bool                in_initialize_plugin;

	/**
	 * The platform services available to plugins
	 */
	PF_PlatformServices platform_services;

	/**
	 * The map of loaded shared library
	 */
	DynamicLibraryMap   dl_map;

	/**
	 * The vector of registered object exit functions
	 */
	ExitFuncVec         exit_func_vec;

	/**
	 * The temporary map of registered object with an associated name (i.e. name is not "*")
	 */
	RegistrationMap     temp_exact_match_map;

	/**
	 * The temporary map of registered object without an associated name (i.e. name is "*")
	 */
	RegistrationVec     temp_wild_vard_vec;

	/**
	 * The map of registered object with an associated name (i.e. name is not "*")
	 */
	RegistrationMap     exact_match_map;

	/**
	 * The map of registered object without an associated name (i.e. name is "*")
	 */
	RegistrationVec     wild_card_vec;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_PLUGIN_MANAGER_H_

