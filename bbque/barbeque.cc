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

#include "bbque/barbeque.h"

#include "bbque/configuration_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/platform_services.h"
#include "bbque/plugin_manager.h"
#include "bbque/resource_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/signals_manager.h"
#include "bbque/test_platform_data.h"

#include "bbque/utils/timer.h"
#include "bbque/utils/utility.h"

#include "bbque/plugins/test.h"

namespace bb = bbque;
namespace bp = bbque::plugins;
namespace bu = bbque::utils;
namespace po = boost::program_options;

#define FMT(fmt) BBQUE_FMT(COLOR_GREEN, "BQ", fmt)


/* The global timer, this can be used to get the time since Barbeque start */
bu::Timer bbque_tmr(true);

int Tests(bp::PluginManager & pm) {
	const bp::PluginManager::RegistrationMap & rm = pm.GetRegistrationMap();
	bp::PluginManager::RegistrationMap::const_iterator near_match =
		rm.lower_bound(TEST_NAMESPACE);

	if (near_match == rm.end() ||
			((*near_match).first.compare(0,
				strlen(TEST_NAMESPACE),TEST_NAMESPACE)))
		return false;

	fprintf(stdout, FMT(".:: Entering Testing Mode\n"));

	do {
		bu::Timer test_tmr;

		fprintf(stdout, "\n"FMT("___ Testing [%s]...\n"),
				(*near_match).first.c_str());

		bp::TestIF * tms = bb::ModulesFactory::GetTestModule(
				(*near_match).first);

		test_tmr.start();
		tms->Test();
		test_tmr.stop();

		fprintf(stdout, FMT("___ completed, [%11.6f]s\n"),
				test_tmr.getElapsedTime());

		near_match++;

	} while (near_match != rm.end() &&
			((*near_match).first.compare(0,5,"test.")) == 0 );

	fprintf(stdout, "\n\n"FMT(".:: All tests completed\n\n"));
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {

	// Command line parsing
	bb::ConfigurationManager & cm = bb::ConfigurationManager::GetInstance();
	cm.ParseCommandLine(argc, argv);

	// Welcome screen
	fprintf(stdout, FMT(".:: Barbeque RTRM (ver. %s) ::.\n"),
			g_git_version);
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

#if BBQUE_TEST_PLATFORM_DATA
	//---------- Loading TEST platform data
	// This is just a temporary placeholder for the (still-missing)
	// ResourceAbstraction module
	bb::TestPlatformData &tpd(bb::TestPlatformData::GetInstance());
	tpd.LoadPlatformData();
#endif // BBQUE_TEST_PLATFORM_DATA

	// Initialize Signals Manager module
	bb::SignalsManager::GetInstance();

	// Check if we have tests to run
	if (cm.RunTests())
		return Tests(pm);

	// Let's start baking applications...
	bb::ResourceManager::GetInstance().Go();


	// Cleaning-up the grill


	return EXIT_SUCCESS;

}

