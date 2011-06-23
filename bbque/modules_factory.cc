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

#include "bbque/console_logger.h"

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

plugins::TestIF * ModulesFactory::GetTestModule(const std::string & id) {
	// Ensure ModulesFactory initialization
	ModulesFactory::GetInstance();
	// Build a object adapter for the TestModule
	Test_ObjectAdapter toa;

	void * module = bp::PluginManager::GetInstance().
						CreateObject(id, NULL, &toa);

	return (plugins::TestIF *) module;
}

/**
 * Specialize the ObjectAdapter template for Logger plugins
 */
typedef bp::ObjectAdapter<bp::LoggerAdapter, C_Logger> Logger_ObjectAdapter;

plugins::LoggerIF * ModulesFactory::GetLoggerModule(
		plugins::LoggerIF::Configuration const & data,
		std::string const & id) {
	std::shared_ptr<bp::ConsoleLogger> logger;

	// Ensure ModulesFactory initialization
	ModulesFactory::GetInstance();
	// Build a object adapter for the Logger
	Logger_ObjectAdapter loa;

	void * module = bp::PluginManager::GetInstance().
		CreateObject(id, (void*)&data, &loa);

	// Since this is a critical module, if the logger modules is not able to
	// successifully load, we fall-back to a dummy (console based) logger
	// implementation.
	if (!module) {
		logger = bp::ConsoleLogger::GetInstance();
		logger->Error("Logger module loading/configuration FAILED");
		logger->Warn("Using (dummy) console logger");
		module = (void*)(logger.get());
	}

	return (plugins::LoggerIF *) module;
}

plugins::RPCChannelIF * ModulesFactory::GetRPCChannelModule(
    std::string const & id) {
	return RPCProxy::GetInstance(id);
}

plugins::RecipeLoaderIF * ModulesFactory::GetRecipeLoaderModule(
    std::string const & id) {

	// Ensure ModulesFactory initialization
	ModulesFactory::GetInstance();

	// RecipeLoader is just implemented in C++ thus it doesn't
	// require a real ObjectAdapter
	void * module = bp::PluginManager::GetInstance().
	                CreateObject(id);

	return (plugins::RecipeLoaderIF *) module;
}

plugins::SchedulerPolicyIF * ModulesFactory::GetSchedulerPolicyModule(
		std::string const & id) {

	// Ensure ModulesFactory initialization
	ModulesFactory::GetInstance();

	// SchedulerPolicy is just implemented in C++ thus it doesn't
	// require a real ObjectAdapter
	void * module = bp::PluginManager::GetInstance().
						CreateObject(id);

	return (plugins::SchedulerPolicyIF *) module;
}

plugins::SynchronizationPolicyIF * ModulesFactory::GetSynchronizationPolicyModule(
		std::string const & id) {

	// Ensure ModulesFactory initialization
	ModulesFactory::GetInstance();

	// SchedulerPolicy is just implemented in C++ thus it doesn't
	// require a real ObjectAdapter
	void * module = bp::PluginManager::GetInstance().
						CreateObject(id);

	return (plugins::SynchronizationPolicyIF *) module;
}
} // namespace bbque

