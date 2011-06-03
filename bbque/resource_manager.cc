/**
 *       @file  resource_manager.cc
 *      @brief  The Barbeque Run-Time Resource Manager
 *
 * This class provides the implementation of the Run-Time Resource Manager
 * (RTRM), which is the main barbeque module implementing its glue code.
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
 * =============================================================================
 */

#include "bbque/resource_manager.h"

#include "bbque/configuration_manager.h"
#include "bbque/plugin_manager.h"
#include "bbque/modules_factory.h"

#include "bbque/utils/utility.h"

namespace bp = bbque::plugins;
namespace po = boost::program_options;

namespace bbque {

ResourceManager & ResourceManager::GetInstance() {
	static ResourceManager rtrm;

	return rtrm;
}

ResourceManager::ResourceManager() :
	ps(PlatformServices::GetInstance()),
	pm(plugins::PluginManager::GetInstance()),
	am(ApplicationManager::GetInstance()),
	ap(ApplicationProxy::GetInstance()) {

}

ResourceManager::~ResourceManager() {
}

void ResourceManager::Setup() {

	//---------- Get a logger module
	std::string logger_name("bq.rm");
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "RM: Logger module creation FAILED\n");
		assert(logger);
	}

	//---------- Dump list of registered plugins
	const bp::PluginManager::RegistrationMap & rm = pm.GetRegistrationMap();
	logger->Info("RM: Registered plugins:");
	bp::PluginManager::RegistrationMap::const_iterator i;
	for (i = rm.begin(); i != rm.end(); ++i)
		logger->Info(" * %s", (*i).first.c_str());

	//---------- Start bbque services
	ap.Start();
}

void ResourceManager::ControlLoop() {

	DB(
	ConfigurationManager & cm = ConfigurationManager::GetInstance();
	uint16_t test_run = cm.GetOptions()["debug.test_time"].as<uint16_t>();

	// Fake control loop implementation
	std::cerr << "Exiting in " << test_run << "s..." << std::endl;
	::sleep(test_run);
	);

	done = true;
}

void ResourceManager::Go() {

	Setup();

	while (!done) {
		ControlLoop();
	}

}

} // namespace bbque

