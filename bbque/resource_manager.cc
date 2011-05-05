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
 * ============================================================================
 */

#include "bbque/resource_manager.h"

#include "bbque/configuration_manager.h"
#include "bbque/plugin_manager.h"
#include "bbque/modules_factory.h"

#include "bbque/plugins/logger.h"

namespace bp = bbque::plugins;
namespace po = boost::program_options;

namespace bbque {

ResourceManager & ResourceManager::GetInstance() {
	static ResourceManager rtrm;

	return rtrm;
}

ResourceManager::ResourceManager() :
	ps(PlatformServices::GetInstance()),
	pm(plugins::PluginManager::GetInstance()) {

}

ResourceManager::~ResourceManager() {
}

void ResourceManager::Go() {


	// Dump a list of registered plugins
	const bp::PluginManager::RegistrationMap & rm = pm.GetRegistrationMap();
	fprintf(stdout, "RM: Registered plugins:\n");
	for (bp::PluginManager::RegistrationMap::const_iterator i = rm.begin(); i != rm.end(); ++i) {
	// Dump all the loaded modules
		fprintf(stdout, "    * %s\n", (*i).first.c_str());
	}

	//---------- JustForTest: Static Module
	fprintf(stdout, "\nRM: Looking for nearest matching module [test.]\n");
	// Build a Test object
	plugins::TestIF * tms = ModulesFactory::GetTestModule();
	if (tms) {
		fprintf(stdout, "RM: Found a module within namespace '"
				TEST_NAMESPACE "'\n");
		tms->Test();
	} else {
		fprintf(stdout, "RM: Unable to find a module within namespace '"
				TEST_NAMESPACE "'\n");
	}

	//---------- JustForTest: Dynamic Module
	fprintf(stdout, "\nRM: Looking for (nearest matching) module"
			"[test.dummy_dyn]\n");
	// Build a Test object
	plugins::TestIF * tmd = ModulesFactory::GetTestModule("test.dummy_dyn");
	if (tmd) {
		fprintf(stdout, "RM: Found a module within namespace '"
				TEST_NAMESPACE "'\n");
		tmd->Test();
	} else {
		fprintf(stdout, "RM: Unable to find a module within namespace '"
				TEST_NAMESPACE "'\n");
	}

	//---------- Get a logger module
	fprintf(stdout, "\nRM: Looking for nearest matching module [logger.]\n");
	// Build a Logger object
	std::string logger_name("bq.rm");
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	plugins::LoggerIF * logger =
		ModulesFactory::GetLoggerModule(std::cref(conf));
	if (logger) {
		fprintf(stdout, "RM: Found a module within namespace '"
				TEST_NAMESPACE "'\n");
		logger->Debug("Test DEBUG message");
		logger->Info("Test INFO message");
		logger->Warn("Test WARN message");
	} else {
		fprintf(stdout, "RM: Unable to find a module within namespace '"
				LOGGER_NAMESPACE "'\n");
	}
	std::cout << "\n" << std::endl;

	while (!done) {
		ControlLoop();
	}

}

void ResourceManager::ControlLoop() {
#ifdef BBQUE_DEBUG
	ConfigurationManager & cm = ConfigurationManager::GetInstance();
	uint16_t test_run = cm.GetOptions()["debug.test_time"].as<uint16_t>();

	// Fake control loop implementation
	std::cerr << "Exiting in " << test_run << "s..." << std::endl;
	::sleep(test_run);
#endif

	done = true;
}

} // namespace bbque

