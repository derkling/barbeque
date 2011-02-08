/**
 *       @file  configuration_manager.h
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

#ifndef BBQUE_CONFIGURATION_MANAGER_H_
#define BBQUE_CONFIGURATION_MANAGER_H_

#include "bbque/barbeque.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
namespace po = boost::program_options;

namespace bbque {

class ConfigurationManager {

public:

	~ConfigurationManager();

	static ConfigurationManager & GetInstance();

	void ParseCommandLine(int argc, char *argv[]);

	void ParseConfigurationFile(
			po::options_description const & opts_desc,
			po::variables_map & opts);

	inline po::variables_map const & GetOptions() const {
		return std::cref(opts_vm);
	}

	inline bool LoadPlugins() const {
		return opts_vm.count("plugins");
	}

	inline std::string GetPluginsDir() const {
		return plugins_dir;
	}


private:

	ConfigurationManager();

	po::options_description core_opts_desc;

	po::options_description all_opts_desc;

#ifdef BBQUE_DEBUG
	po::options_description dbg_opts_desc;
	uint16_t test_run;
#endif

	po::options_description cmd_opts_desc;

	po::variables_map opts_vm;

	std::string conf_file_path;

	std::string plugins_dir;

};

} // namespace bbque

#endif // BBQUE_CONFIGURATION_MANAGER_H_

