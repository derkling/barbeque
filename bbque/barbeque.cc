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

#include "bbque/utils/timer.h"
#include "bbque/utils/utility.h"

#include "bbque/plugins/test.h"

namespace bb = bbque;
namespace bp = bbque::plugins;
namespace bu = bbque::utils;
namespace po = boost::program_options;

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

	fprintf(stdout, FMT_INFO("Entering Testing Mode\n"));

	do {
		bu::Timer test_tmr;

		fprintf(stdout, "\n"FMT_INFO("___ Testing [%s]...\n"),
				(*near_match).first.c_str());

		bp::TestIF * tms = bb::ModulesFactory::GetTestModule(
				(*near_match).first);

		test_tmr.start();
		tms->Test();
		test_tmr.stop();

		fprintf(stdout, FMT_INFO("___ completed, [%11.6f]s\n"),
				test_tmr.getElapsedTime());

		near_match++;

	} while (near_match != rm.end() &&
			((*near_match).first.compare(0,5,"test.")) == 0 );

	fprintf(stdout, "\n\n"FMT_INFO("All tests completed\n\n"));
	return EXIT_SUCCESS;
}

// The deamonizing ruotine
extern int
daemonize(const char *name, const char *uid,
		const char *lockfile, const char *pidfile,
		const char *rundir);

static void DaemonizeBBQ(bb::ConfigurationManager & cm) {
	int result;

	fprintf(stderr, "Starting BarbequeRTRM as daemon [%s], running with uid [%s]...\n",
			cm.GetDaemonName().c_str(), cm.GetUID().c_str());
	fprintf(stderr, "Using configuration file [%s]\n",
			cm.GetConfigurationFile().c_str());

	// Daemonize this process
	result = daemonize(
			cm.GetDaemonName().c_str(),
			cm.GetUID().c_str(),
			cm.GetLockfile().c_str(),
			cm.GetPIDfile().c_str(),
			cm.GetRundir().c_str()
			);
	if (result == 0) {
		// The OK is not reported, since actual BBQ initialization
		// could still fails
		return;
	}

	syslog(LOG_ERR, "Daemonization FAILED, terminate BBQ");
	fprintf(stderr, "FAILED\n");
	exit(EXIT_FAILURE);

}

int main(int argc, char *argv[]) {
	int exit_code;

	/* Initialize the logging interface */
	openlog(BBQUE_DAEMON_NAME, LOG_PID, LOG_LOCAL5);

	// Command line parsing
	bb::ConfigurationManager & cm = bb::ConfigurationManager::GetInstance();
	cm.ParseCommandLine(argc, argv);

	// Check if we should run as daemon
	if (cm.RunAsDaemon()) {
		syslog(LOG_INFO, "Starting BBQ daemon (ver. %s)...", g_git_version);
		syslog(LOG_INFO, "BarbequeRTRM build time: " __DATE__  " " __TIME__);
		DaemonizeBBQ(cm);
	} else {
		// Welcome screen
		fprintf(stdout, FMT_INFO("Starting BBQ (ver. %s)...\n"), g_git_version);
		fprintf(stdout, FMT_INFO("BarbequeRTRM build time: " __DATE__  " " __TIME__ "\n"));
	}

	// Initialization
	bp::PluginManager & pm = bp::PluginManager::GetInstance();
	pm.GetPlatformServices().InvokeService =
		bb::PlatformServices::ServiceDispatcher;

	// Plugins loading
	if (cm.LoadPlugins()) {
		if (cm.RunAsDaemon())
			syslog(LOG_INFO, "Loading plugins from dir [%s]...",
					cm.GetPluginsDir().c_str());
		else
			fprintf(stdout, FMT_INFO("Loading plugins from dir [%s]...\n"),
					cm.GetPluginsDir().c_str());
		pm.LoadAll(cm.GetPluginsDir());
	}

	// Initialize Signals Manager module
	bb::SignalsManager::GetInstance();

	// Check if we have tests to run
	if (cm.RunTests())
		return Tests(pm);

	// Let's start baking applications...
	bb::ResourceManager::ExitCode_t result =
		bb::ResourceManager::GetInstance().Go();
	if (result != bb::ResourceManager::ExitCode_t::OK) {
		exit_code = EXIT_FAILURE;
		goto bbq_exit;
	}

bbq_exit:

	// Cleaning-up the grill

	// Final greetings
	if (cm.RunAsDaemon())
		syslog(LOG_INFO, "BBQ daemon termination [%s]",
				(exit_code == EXIT_FAILURE) ? "FAILURE" : "SUCCESS" );
	else
		fprintf(stdout, FMT_INFO("BBQ termination [%s]\n"),
				(exit_code == EXIT_FAILURE) ? "FAILURE" : "SUCCESS" );

	closelog();
	return exit_code;
}

