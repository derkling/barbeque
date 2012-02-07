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

