/**
 *       @file  test_platform_data.cc
 *      @brief  A dummy loader of test platform data
 *
 * This file provides a descritpion of platform resoruces suitable for testing
 * purposes while a real implementation of the ResourceAbstraction module is
 * missing. The platform defined by this module can be configured using
 * command lines parameters to define the number of clusters, PEs for each
 * cluster, the amount of cluster-shared memory and the amount of PE-private
 * memory.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  06/22/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_TEST_PLATFORM_DATA_H_
#define BBQUE_TEST_PLATFORM_DATA_H_

#include "bbque/plugins/logger.h"
#include "bbque/plugin_manager.h"


#define TEST_PLATFORM_DATA_NAMESPACE "bq.tpd"

namespace bbque {

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

