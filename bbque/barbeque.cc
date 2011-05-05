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
#include "bbque/modules_factory.h"

#include "bbque/utils/timer.h"
#include "bbque/utils/utility.h"

namespace bb = bbque;
namespace bp = bbque::plugins;
namespace bu = bbque::utils;
namespace po = boost::program_options;

#define FMT(fmt) BBQUE_FMT(COLOR_GREEN, "BQ", fmt)


/* The global timer, this can be used to get the time since Barbeque start */
bu::Timer bbque_tmr(true);

bool runTests(bp::PluginManager & pm) {
	const bp::PluginManager::RegistrationMap & rm = pm.GetRegistrationMap();
	bp::PluginManager::RegistrationMap::const_iterator near_match =
		rm.lower_bound("test.");

	if (near_match == rm.end() ||
			((*near_match).first.compare(0,5,"test.")))
		return false;

	fprintf(stdout, FMT(".:: Entering Testing Mode\n"));

	do {
		bu::Timer test_tmr;

		fprintf(stdout, "\n"FMT("___ Testing [%s]...\n"), (*near_match).first.c_str());

		bp::TestIF * tms = bb::ModulesFactory::GetTestModule((*near_match).first);

		test_tmr.start();
		tms->Test();
		test_tmr.stop();

		fprintf(stdout, FMT("___ completed, [%11.6f]s\n"), test_tmr.getElapsedTime());

		near_match++;

	} while (near_match != rm.end() &&
			((*near_match).first.compare(0,5,"test.")) == 0 );

	fprintf(stdout, "\n\n"FMT(".:: All tests completed\n\n"));
	return true;
}

int main(int argc, char *argv[]) {

	// Command line parsing
	bb::ConfigurationManager & cm = bb::ConfigurationManager::GetInstance();
	cm.ParseCommandLine(argc, argv);

	// Welcome screen
	fprintf(stdout, FMT(".:: Barbeque RTRM (ver. %s) ::.\n"), g_git_version);
	fprintf(stdout, FMT("Built: " __DATE__  " " __TIME__ "\n\n"));

	// Initialization
	bp::PluginManager & pm = bp::PluginManager::GetInstance();
	pm.GetPlatformServices().InvokeService =
		bb::PlatformServices::ServiceDispatcher;

	// Plugins loading
	if (cm.LoadPlugins()) {
		fprintf(stdout, FMT("Loading plugins, dir [%s]\n"),
				cm.GetPluginsDir().c_str());
		pm.LoadAll(cm.GetPluginsDir());
	}

	// Check if we have tests to run (i.e. "test." objects have been registered)
	if (runTests(pm))
		return EXIT_SUCCESS;

	// Let's start baking applications...
	bb::ResourceManager::GetInstance().Go();


	// Cleaning-up the grill


	return EXIT_SUCCESS;

}

