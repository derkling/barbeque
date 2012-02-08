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

#ifndef BBQUE_APROX_TEST_H_
#define BBQUE_APROX_TEST_H_

#include "bbque/object.h"
#include "bbque/plugins/plugin.h"
#include "bbque/plugins/test.h"

#include "bbque/plugins/logger.h"

#define APROX_NAMESPACE "aprox"

using bbque::plugins::LoggerIF;

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

/**
 * @brief A TEST for the ApplicationProxy module
 *
 * A class to test the implementation of the ApplicationPorxy
 * module. This test evaluate both functional and performance aspects of the
 * implementation.
 * It embeds a set of calls for testing the managing of an application
 * lifecycle inside Barbeque RTRM. The class is the core of a plugin is used
 * for testing purpose.
 */
class ApplicationProxyTest: public TestIF {

public:

	/**
	 * @brief Plugin creation method
	 */
	static void * Create(PF_ObjectParams *);

	/**
	 * @brief Plugin destruction method
	 */
	static int32_t Destroy(void *);

	/**
	 * @brief class destructor
	 */
	~ApplicationProxyTest();

	/**
	 * @brief Test launcher
	 */
	void Test();

private:

	std::unique_ptr<LoggerIF> logger;

	/**
	 * @brief Constructor
	 */
	ApplicationProxyTest();

	int RegisterSomeResources();

	int PrintResourceAvailabilities();

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_APROX_TEST_H_
