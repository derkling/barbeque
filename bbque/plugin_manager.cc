/**
 *       @file  plugin_manager.cc
 *      @brief  The Barbeque RTRM Plugin Manager
 *
 * This provides the definition of the Puligin Manager for the Barbeque RTRM.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/27/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include "bbque/plugin_manager.h"

#include <boost/filesystem.hpp>
#include <memory>
#include <string>

#include "bbque/dynamic_library.h"
#include "bbque/plugins/object_adapter.h"

namespace fs = boost::filesystem;

namespace bbque { namespace plugins {

static std::string dl_ext(".so");

PluginManager::PluginManager() :
	in_initialize_plugin(false) {

	// Setup platform services data
	platform_services.version.major = 1;
	platform_services.version.minor = 0;

	// can be populated during loadAll()
	platform_services.InvokeService = NULL;
	platform_services.RegisterObject = RegisterObject;

}

PluginManager::~PluginManager() {
	// Just in case it wasn't called earlier, here we ensure that all plugins
	// release methods have been called thus properly releasing all resources.
	Shutdown();
}

PluginManager & PluginManager::GetInstance() {
  static PluginManager instance;

  return instance;
}

/**
 * @brief   Check plugins data initialization
 *
 * The registration params may be received from an external plugin so it is
 * crucial to validate it, because it was never subjected to our tests.
 *
 * @param   id the pointer to the plugins object type string
 * @param	params the plugins data
 * @return  true if data is correctly initialized, false otherwise
 */
static bool isValid(const char * id, const PF_RegisterParams * params) {
	if (!id || !(*id))
		return false;
	if (!params ||!params->CreateFunc || !params->DestroyFunc)
		return false;

	return true;
}

int32_t PluginManager::RegisterObject(const char * id, const PF_RegisterParams * params) {

	// Check parameters
	if (!isValid(id, params))
		return -1;

	PluginManager & pm = PluginManager::GetInstance();

	// Verify that versions match
	PF_PluginAPIVersion v = pm.platform_services.version;
	if (v.major != params->version.major) {
		fprintf(stderr, "PM: Module [%s] version mismatching\n", id);
		return -2;
	}

	std::string key((const char *)id);
	if (key == std::string("*")) {
		// If it's a wild card registration just add it
		pm.wild_card_vec.push_back(*params);
		return 0;
	}

	if (pm.exact_match_map.find(key) != pm.exact_match_map.end()) {
		// item already exists in exact_match_map fail (only one can handle)
		fprintf(stderr, "PM: Module [%s] already registered\n", id);
		return -3;
	}

	pm.exact_match_map[key] = *params;
	//fprintf(stdout, "PM: New module [%s:%p] registered\n",
	//	key.c_str(), (void*)params->CreateFunc);
	return 0;
}

int32_t PluginManager::LoadAll(const std::string & pluginDir, PF_InvokeServiceFunc func) {
	fs::path plugins_dir(pluginDir);

	if (pluginDir.empty()) {
		// The path is empty
		fprintf(stderr, "PM: Empty plugins dir [%s]\n", pluginDir.c_str());
		return -1;
	}

	if (func!=NULL)
		platform_services.InvokeService = func;

	if (!fs::exists(plugins_dir) || !fs::is_directory(plugins_dir))
		return -2;

	fs::directory_iterator it(plugins_dir);
	fs::directory_iterator end; // default construction yields past-the-end
	for ( ; it != end; ++it ) {

		if ( is_directory(it->status()) ) {
			// Skip sub-directories
			continue;
		}

		std::string ext = fs::extension(it->path());
		if ( ext != dl_ext ) {
			// Skip files with the wrong extension
			continue;
		}

		// Ignore return value (int32_t res)
		LoadByPath(it->path().string());
	}

	return 0;
}

int32_t PluginManager::InitializePlugin(PF_InitFunc initFunc) {
	PluginManager & pm = PluginManager::GetInstance();

	PF_ExitFunc exitFunc = initFunc(&pm.platform_services);
	if (!exitFunc)
		return -1;

	// Store the exit func so it can be called when unloading this plugin
	pm.exit_func_vec.push_back(exitFunc);
	return 0;
}

int32_t PluginManager::Shutdown() {
	int32_t result = 0;

	for (ExitFuncVec::iterator func = exit_func_vec.begin();
			func != exit_func_vec.end(); ++func) {
		try {
			result = (*func)();
		} catch (...) {
			// TODO: check error, we should go on with the remaining plugins!?!
			result = -1;
		}
	}

	dl_map.clear();
	exact_match_map.clear();
	wild_card_vec.clear();
	exit_func_vec.clear();

	return result;
}

int32_t PluginManager::LoadByPath(const std::string & pluginPath) {
	fs::path path(pluginPath);

	// Resolve symbolic links
	if (fs::is_symlink(path)) {

		// loop 'til buffer large enough
		for (long path_max = 64; ; path_max*=2) {
			ssize_t result;
			std::unique_ptr<char> buf(new char[static_cast<std::size_t>(path_max)]);

			result = ::readlink(path.string().c_str(), buf.get(),
					static_cast<std::size_t>(path_max));
			if (result == -1 ) {
				return errno;
			}
			if(result == path_max)
				continue;

			path = fs::path(buf.get());
			break;
		}
	}

	// Don't load the same dynamic library twice
	if (dl_map.find(path.string()) != dl_map.end())
		return -1;

	fprintf(stdout, "PM: Loading plugin [%s]\n", pluginPath.c_str());

	std::string errorString;
	DynamicLibrary * dl = LoadLibrary(fs::system_complete(path).string(),
					errorString);
	if (!dl) {
		// not a dynamic library
		return -1;
	}

	// Get the NTA_initPlugin() function
	PF_InitFunc initFunc = (PF_InitFunc)(dl->GetSymbol("PF_initPlugin"));
	if (!initFunc) {
		// missing dynamic library entry point
		fprintf(stderr, "PM: Missing [PF_initPlugin] plugin entry point");
		return -1;
	}

	int32_t res = InitializePlugin(initFunc);
	if (res < 0) {
		// initialization failed
		fprintf(stderr, "PM: Initialization failed");
		return res;
	}

	return 0;
}

void * PluginManager::CreateObject(const std::string & id,
		ObjectAdapterIF & adapter, void * data) {
	// "*" is not a valid object type
	if (id == std::string("*"))
		return NULL;

	// Prepare object params
	PF_ObjectParams op;
	op.id = (const char *)id.c_str();
	op.data = data;
	op.platform_services = &platform_services;

	// Try to find a lower bound match (i.e. an object within the specified
	// namespace), e.g. "logger." will match "logger.console"
	RegistrationMap::iterator near_match = exact_match_map.lower_bound(id);
	if ( near_match != exact_match_map.end() &&
			((*near_match).first.compare(0,id.size(),id)) == 0 ) {

		fprintf(stdout, "PM: Found matching module [%s@%p]\n",
			(*near_match).first.c_str(),
			(void*)(*near_match).second.CreateFunc);

		// Class (or full) match found
		PF_RegisterParams & rp = (*near_match).second;
		void * object = rp.CreateFunc(&op);
		if (object) {
			// Great, there is an exact match
			// Adapt if necessary (wrap C objects using an adapter)
			if (rp.programming_language == PF_LANG_C)
				object = adapter.adapt(object, rp.DestroyFunc);

			return object;
		}

	}

	// Try to find a wild card match
	for (size_t i = 0; i < wild_card_vec.size(); ++i) {

		PF_RegisterParams & rp = wild_card_vec[i];
		void * object = rp.CreateFunc(&op);
		if (!object)
			continue;

		// Great, match found!

		// Adapt if necessary (wrap C objects using an adapter)
		if (rp.programming_language == PF_LANG_C) {
			object = adapter.adapt(object, rp.DestroyFunc);

			// promote registration to exact_matc
			// (but keep also the wild card registration for other object types)
			int32_t res = RegisterObject(op.id, &rp);
			if (res < 0) {
				// TODO: we should report or log it
				rp.DestroyFunc(object);
				return NULL;
			}

			return object;
		}
	}

	// Too bad no one can create this id
	return NULL;
}

DynamicLibrary * PluginManager::LoadLibrary(const std::string & path, std::string & errorString) {
	DynamicLibrary * dl = DynamicLibrary::Load(path, errorString);
	if (!dl) {
		// not a dynamic library?
		return NULL;
	}

	// Add library to map, so it can be unloaded
	dl_map[path] = std::shared_ptr<DynamicLibrary>(dl);
	return dl;
}

PF_ProgrammingLanguage PluginManager::GetModuleLanguage(
		const std::string & id) {

	// "*" is not a valid object type
	if (id == std::string("*"))
		return PF_LANG_UNDEF;

	if (exact_match_map.find(id) != exact_match_map.end()) {
		// Exact match found
		PF_RegisterParams & rp = exact_match_map[id];
		return rp.programming_language;
	}

	return PF_LANG_UNDEF;
}

const PluginManager::RegistrationMap & PluginManager::GetRegistrationMap() {
	return exact_match_map;
}

PF_PlatformServices & PluginManager::GetPlatformServices() {
	return platform_services;
}

} // namespace plugins

} // namespace bbque

