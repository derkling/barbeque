/**
 *       @file  modules_factory.cc
 *      @brief  A modules factory class
 *
 * This provides a factory of barbeuque modules. Each module could be build by
 * the core framework thanks to a correponfing method of this singleton
 * Factory class.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  02/01/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/modules_factory.h"

#include "bbque/plugin_manager.h"
#include "bbque/plugins/object_adapter.h"

namespace bp = bbque::plugins;

namespace bbque {

ModulesFactory::ModulesFactory() {
}

ModulesFactory & ModulesFactory::GetInstance() {
	static ModulesFactory instance;
	return instance;
}


/**
 * Specialize the ObjectAdapter template for Test plugins
 */
typedef bp::ObjectAdapter<bp::TestAdapter, C_Test> Test_ObjectAdapter;

plugins::TestIF * ModulesFactory::GetTestModule(const std::string & objectType) {
	// Ensure ModulesFactory initialization
	ModulesFactory::GetInstance();
	// Build a object adapter for the TestModule
	Test_ObjectAdapter toa;

	void * module = bp::PluginManager::GetInstance().
						CreateObject(objectType, toa);

	return (plugins::TestIF *) module;
}

/**
 * Specialize the ObjectAdapter template for Logger plugins
 */
typedef bp::ObjectAdapter<bp::LoggerAdapter, C_Logger> Logger_ObjectAdapter;

plugins::LoggerIF * ModulesFactory::GetLoggerModule(const std::string & objectType) {
	// Ensure ModulesFactory initialization
	ModulesFactory::GetInstance();
	// Build a object adapter for the Logger
	Logger_ObjectAdapter loa;

	void * module = bp::PluginManager::GetInstance().
						CreateObject(objectType, loa);

	return (plugins::LoggerIF *) module;
}

} // namespace bbque

