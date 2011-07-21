/**
 *       @file  configuration_manager.cc
 *      @brief  Provides the inteface towards configurations, either on
 *      command line or configuration file.
 *
 * This class defines the set of methods to access Barbeque run-time
 * configuration options, either on command line and configuration file.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  02/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/configuration_manager.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "bbque/barbeque.h"

namespace po = boost::program_options;

namespace bbque {

ConfigurationManager::ConfigurationManager() :
	core_opts_desc("Generic Options"),
	all_opts_desc(""),
#ifdef BBQUE_DEBUG
	//dbg_opts_desc("Debugging Options"),
#endif
#ifdef BBQUE_TEST_PLATFORM_DATA
	tpd_opts_desc("TEST Platform Data Options"),
#endif // BBQUE_TEST_PLATFORM_DATA
	cmd_opts_desc("") {

	core_opts_desc.add_options()
		("help,h", "print this help message")
		("config,c", po::value<std::string>(&conf_file_path)->
			default_value("/etc/bbque.conf"),
			"configuration file path")
		("bbque.plugins,p", po::value<std::string>(&plugins_dir)->
			default_value(BBQUE_PATH_PREFIX"/"BBQUE_PATH_PLUGINS),
			"plugins folder")
		("bbque.test,t", "Run TESTs plugins")
		("version,v", "print program version")
		;
	all_opts_desc.add(core_opts_desc);
	cmd_opts_desc.add(core_opts_desc);

#ifdef BBQUE_DEBUG
	//dbg_opts_desc.add_options()
	//	("debug.test_time", po::value<uint16_t>(&test_run)->
	//		default_value(5),
	//		"how long [s] to run")
	//	;
	//all_opts_desc.add(dbg_opts_desc);
	//cmd_opts_desc.add(dbg_opts_desc);
#endif

#ifdef BBQUE_TEST_PLATFORM_DATA
	tpd_opts_desc.add_options()
		("tpd.clusters", po::value<unsigned short>(&tpd_clusters_count)->
			default_value(3),
			"number of clusters (1..256, default: 3)")
		("tpd.cmem", po::value<uint16_t>(&tpd_cluster_mem_mb)->
			default_value(8120),
			"amount [MB] of cluster-shared memory (1..65536, default: 8120MB)")
		("tpd.pes", po::value<unsigned short>(&tpd_pes_count)->
			default_value(4),
			"number of PEs per cluster (1..256, default: 4)")
		;
	all_opts_desc.add(tpd_opts_desc);
	cmd_opts_desc.add(tpd_opts_desc);
#endif // BBQUE_TEST_PLATFORM_DATA

}

ConfigurationManager::~ConfigurationManager() {

}

ConfigurationManager & ConfigurationManager::GetInstance() {
	static ConfigurationManager instance;
	return instance;
}

void ConfigurationManager::ParseCommandLine(int argc, char *argv[]) {

	// Parse command line params
	try {
	po::store(po::parse_command_line(argc, argv, cmd_opts_desc), opts_vm);
	} catch(...) {
		std::cout << "Usage: " << argv[0] << " [options]\n";
		std::cout << cmd_opts_desc << std::endl;
		::exit(EXIT_FAILURE);
	}
	po::notify(opts_vm);

	// Check for help request
	if (opts_vm.count("help")) {
		std::cout << "Usage: " << argv[0] << " [options]\n";
		std::cout << cmd_opts_desc << std::endl;
		::exit(EXIT_SUCCESS);
	}

	// Check for version request
	if (opts_vm.count("version")) {
		std::cout << "Barbeque RTRM (ver. " << g_git_version << ")\n";
		std::cout << "Copyright (C) 2011 Politecnico di Milano\n";
		std::cout << "\n";
		std::cout << "Built on " << __DATE__ << " " << __TIME__ << "\n";
		std::cout << "\n";
		std::cout << "This is free software; see the source for copying conditions.  There is NO\n";
		std::cout << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.";
		std::cout << "\n" << std::endl;
		::exit(EXIT_SUCCESS);
	}

	ParseConfigurationFile(all_opts_desc, opts_vm);

}

void ConfigurationManager::ParseConfigurationFile(
		po::options_description const & opts_desc,
		po::variables_map & opts) {
	std::ifstream in(conf_file_path);

	// Parse configuration file (allowing for unregistered options)
	po::store(po::parse_config_file(in, opts_desc, true), opts);
	po::notify(opts);

}

} // namespace bbque

