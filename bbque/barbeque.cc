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

#include "bbque/configuration_manager.h"
#include "bbque/platform_services.h"
#include "bbque/plugin_manager.h"
#include "bbque/resource_manager.h"

namespace bb = bbque;
namespace bp = bbque::plugins;

int main(int argc, char *argv[]) {

	// Command line parsing
	bb::ConfigurationManager & cm = bb::ConfigurationManager::GetInstance();
	cm.ParseCommandLine(argc, argv);

	// Welcome screen
	std::cout << "\t\t.:: Barbeque RTRM (ver. " << g_git_version << ") ::."
				<< std::endl;
	std::cout << "Built: " << __DATE__ << " " << __TIME__ << std::endl;


	// Initialization
	bp::PluginManager & pm = bp::PluginManager::GetInstance();
	pm.GetPlatformServices().InvokeService =
		bb::PlatformServices::ServiceDispatcher;

	// Plugins loading


	// Let's start baking applications...
	bb::ResourceManager::GetInstance().Go();


	// Cleaning-up the grill


	return EXIT_SUCCESS;

}

