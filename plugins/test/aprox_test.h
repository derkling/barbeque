/**
 *       @file  aprox_test.h
 *      @brief  A test for the ApplicationProxy module
 *
 * This provides a class to test the implementation of the ApplicationPorxy
 * module. This test evaluate both functional and performance aspects of the
 * implementation.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  05/06/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
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
 * @class ApplicationProxyTest
 *
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

