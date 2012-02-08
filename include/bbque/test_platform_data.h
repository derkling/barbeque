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

#ifndef BBQUE_TEST_PLATFORM_DATA_H_
#define BBQUE_TEST_PLATFORM_DATA_H_

#include "bbque/plugins/logger.h"
#include "bbque/plugin_manager.h"


#define TEST_PLATFORM_DATA_NAMESPACE "bq.tpd"

namespace bbque {

/**
 * @brief Testing platform data
 *
 * This class provides a descritpion of platform resoruces suitable for testing
 * purposes while a real implementation of the ResourceAbstraction module is
 * missing. The platform defined by this module can be configured using
 * command lines parameters to define the number of clusters, PEs for each
 * cluster, the amount of cluster-shared memory and the amount of PE-private
 * memory.
 */
class TestPlatformData {

public:

	/**
	 * @brief Class methods result codes
	 */
	enum ExitCode_t {
		TPD_SUCCESS, /** Load successfull */
		TPD_FAILED /** Load failed */
	};

	/**
	 * @brief Get a reference to this class
	 */
	static TestPlatformData &GetInstance();

	/**
	 * @brief Class destructor
	 */
	~TestPlatformData();

	/**
	 * @brief Load a platform configuration
	 */
	ExitCode_t LoadPlatformData();

private:

	/**
	 * @brief The logger used by the resource manager.
	 */
	plugins::LoggerIF *logger;

	/**
	 * @biref Ture if a platform configuration has been successfully loaded
	 */
	bool platformLoaded;

	/**
	 * @brief Class constructor
	 */
	TestPlatformData();

};

} // namespace bbque

#endif // BBQUE_TEST_PLATFORM_DATA
