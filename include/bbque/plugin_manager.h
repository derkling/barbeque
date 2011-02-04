/**
 *       @file  pluginManager.h
 *      @brief  The Barbeque RTRM Plugin Manager
 *
 * This provides the definition of the Puligin Manager for the Barbeque RTRM.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
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

class PluginManager {

	typedef std::map<std::string, std::shared_ptr<DynamicLibrary>> DynamicLibraryMap;

	typedef std::vector<PF_ExitFunc> ExitFuncVec;

	typedef std::vector<PF_RegisterParams> RegistrationVec;

public:

	typedef std::map<std::string, PF_RegisterParams> RegistrationMap;

	static PluginManager & GetInstance();

	static int32_t InitializePlugin(PF_InitFunc initFunc);

	int32_t LoadAll(const std::string & pluginDirectory,
					PF_InvokeServiceFunc func = NULL);

	int32_t LoadByPath(const std::string & path);

	void * CreateObject(const std::string & id,
					ObjectAdapterIF & adapter);

	int32_t Shutdown();

	static int32_t RegisterObject(const char * node_type,
					const PF_RegisterParams * params);

	PF_ProgrammingLanguage GetModuleLanguage(const std::string & id);

	const RegistrationMap & GetRegistrationMap();

	PF_PlatformServices & GetPlatformServices();

private:

	~PluginManager();

	PluginManager();

	PluginManager(const PluginManager &);

	DynamicLibrary * LoadLibrary(const std::string & path, std::string & errorString);

private:

	bool                in_initialize_plugin;

	PF_PlatformServices platform_services;

	DynamicLibraryMap   dl_map;

	ExitFuncVec         exit_func_vec;

	// register exact-match object types
	RegistrationMap     temp_exact_match_map;

	// wild card ('*') object types
	RegistrationVec     temp_wild_vard_vec;

	// register exact-match object types
	RegistrationMap     exact_match_map;

	// wild card ('*') object types
	RegistrationVec     wild_card_vec;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_PLUGIN_MANAGER_H_

