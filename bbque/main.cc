/**
 *       @file  main.cc
 *      @brief  Tyhe RTRM protorype implementation for 2PARMA EU FP7 project
 *
 * Detailed description starts here.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/11/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include "bbque/barbeque.h"

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>

#include "bbque/plugin_manager.h"
namespace bp = bbque::plugins;

#include "bbque/platform_services.h"
namespace bs = bbque;

#include "bbque/resource_manager.h"
namespace bb = bbque;

// Logger configuration
bool g_log_colored = true;
log4cpp::Category & logger = log4cpp::Category::getInstance("bq");
std::string g_log_configuration = "/tmp/bbque.conf";

int main(int argc, char *argv[]) {

	// Command line parsing


	// Welcome screen
	std::cout << "\t\t.:: Barbeque RTRM (ver. " << g_git_version << ") ::."
				<< std::endl;
	std::cout << "Built: " << __DATE__ << " " << __TIME__ << std::endl;


	// Initialization
	bp::PluginManager & pm = bp::PluginManager::GetInstance();
	pm.GetPlatformServices().InvokeService =
		bs::PlatformServices::ServiceDispatcher;

	try {
		std::cout << "Using logger configuration: " << g_log_configuration
					<< std::endl;
		log4cpp::PropertyConfigurator::configure(g_log_configuration);
	} catch(log4cpp::ConfigureFailure& f) {
		std::cout << "Logger configuration failed: " << f.what() << std::endl;
		return EXIT_FAILURE;
	}
	logger.debug("Logger correctly initialized");
	logger.setPriority(log4cpp::Priority::INFO);


	// Plugins loading


	// Let's start baking applications...
	bb::ResourceManager::GetInstance().Go();


	// Cleaning-up the grill


	return EXIT_SUCCESS;

}

