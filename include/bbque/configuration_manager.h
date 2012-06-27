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

#ifndef BBQUE_CONFIGURATION_MANAGER_H_
#define BBQUE_CONFIGURATION_MANAGER_H_

#include "bbque/config.h"
#include "bbque/barbeque.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using boost::program_options::options_description;
using boost::program_options::variables_map;

namespace bbque {

/**
 * @brief A (signleton) class to access Barbeque configuration options.
 *
 * This class describe the configuration manager module which provides a set
 * of methods to load run-time configuration parameters from a configuration
 * file or at command line.  Command line parameters override the
 * corresponding configuration file values.
 */
class ConfigurationManager {

public:

	/**
	 * @brief   Release the configuration manager module
	 */
	~ConfigurationManager();

	/**
	 * @brief   Get a reference to the configuration manager module
	 * @return  A reference to the configuration manager module
	 */
	static ConfigurationManager & GetInstance();

	/**
	 * @brief   Parse command line parameters
	 * @param   argc number of command line parameters
	 * @param	argv command line parameters
	 */
	void ParseCommandLine(int argc, char *argv[]);

	/**
	 * @brief   Parse configuration file
	 * @param   opts_desc the description of supported configuration parameters
	 * @param   opts the map of configuration parameters values returned
	 */
	void ParseConfigurationFile(
			options_description const & opts_desc,
			variables_map & opts);

	/**
	 * @brief   Get a reference to the configuration parameters values map
	 * @return  A reference to the  configuration parameters values map
	 */
	inline variables_map const & GetOptions() const {
		return std::cref(opts_vm);
	}

	/**
	 * @brief   Check if BBQ should run as a daemon
	 * @return  true if BBQ should run as a daemon, false otherwise
	 */
	inline bool RunAsDaemon() const {
		return opts_vm.count("daemon");
	}

	/**
	 * @brief   Check if plugins should be loaded
	 * @return  true if all plugins should be loaded, false otherwise
	 */
	inline bool LoadPlugins() const {
		return opts_vm.count("bbque.plugins");
	}

	/**
	 * @brief   Check if TESTs plugins should be run
	 * @return  true if TESTs should be run, false otherwise
	 */
	inline bool RunTests() const {
		return opts_vm.count("bbque.test");
	}

	/**
	 * @brief  Get the BBQUE configuration file
	 * @return the complete PATH of the used configuration file
	 */
	inline std::string GetConfigurationFile() const {
		return conf_file_path;
	}

	/**
	 * @brief  Get the BBQUE daemon name
	 * @return the name of the process which runs the BBQ daemon
	 */
	inline std::string GetDaemonName() const {
		return daemon_name;
	}

	/**
	 * @brief  Get the BBQUE daemon UID
	 * @return the user ID to run the dameon under
	 */
	inline std::string GetUID() const {
		return daemon_uid;
	}

	/**
	 * @brief  Get the path of BBQUE lockfile
	 * @return the complete path of the lock file
	 */
	inline std::string GetLockfile() const {
		return daemon_lockfile;
	}

	/**
	 * @brief  Get the path of BBQUE pidfile
	 * @return the complete path of the PID file
	 */
	inline std::string GetPIDfile() const {
		return daemon_pidfile;
	}

	/**
	 * @brief  Get the path of BBQUE run directory
	 * @return the complete path of the run directory
	 */
	inline std::string GetRundir() const {
		return daemon_rundir;
	}

	/**
	 * @brief  Get the patch of the plugins folder
	 * @return  the folder containing plugins
	 */
	inline std::string GetPluginsDir() const {
		return plugins_dir;
	}

#ifdef CONFIG_BBQUE_TEST_PLATFORM_DATA
	inline uint8_t  TPD_ClusterCount() const {
		return tpd_clusters_count;
	}
	inline uint16_t TPD_ClusterMem() const {
		return tpd_cluster_mem_mb;
	}
	inline uint8_t  TPD_PEsCount() const {
		return tpd_pes_count;
	}
#endif // CONFIG_BBQUE_TEST_PLATFORM_DATA

private:

	/**
	 * @brief   Build a new configuration manager module
	 */
	ConfigurationManager();

	/**
	 * The decription of each core modules parameters
	 */
	options_description core_opts_desc;

	/**
	 * The description of all supported parameters
	 */
	options_description all_opts_desc;

#ifdef BBQUE_DEBUG
	///**
	// * The description of debugging parameters
	// */
	//options_description dbg_opts_desc;
	//uint16_t test_run;
#endif

#ifdef CONFIG_BBQUE_TEST_PLATFORM_DATA
	/**
	 * The description of TEST platform data parameters
	 */
	options_description tpd_opts_desc;
	unsigned short tpd_clusters_count;
	uint16_t tpd_cluster_mem_mb;
	unsigned short  tpd_pes_count;
#endif // CONFIG_BBQUE_TEST_PLATFORM_DATA

	/**
	 * The description of command line available parameters
	 */
	options_description cmd_opts_desc;

	/**
	 * The map of all parameters values
	 */
	variables_map opts_vm;

	/**
	 * The path of the configuration file
	 */
	std::string conf_file_path;

	/**
	 * The path of the plugins directory
	 */
	std::string plugins_dir;

	/**
	 * The BBQ daemon process name
	 */
	std::string daemon_name;

	/**
	 * The user ID to run the daemon under
	 */
	std::string daemon_uid;

	/**
	 * The path of the locking file
	 */
	std::string daemon_lockfile;

	/**
	 * The path of the PID file
	 */
	std::string daemon_pidfile;

	/**
	 * daemon run directory
	 */
	std::string daemon_rundir;
};

} // namespace bbque

#endif // BBQUE_CONFIGURATION_MANAGER_H_
