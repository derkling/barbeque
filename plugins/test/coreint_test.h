/**
 *       @file  coreint_test.h
 *      @brief  CoreInteractions test plugin
 *
 * It simulates the registering of a set of resources, and the start of an
 * application. Here a recipe is loaded and all the check upon correctness of
 * resource usages defined in are made.
 * Once the ApplicationManager confirms the application has loaded, the test
 * print out their working modes details and then trigger some scheduling
 * status and working mode changes (passing overheads info too).
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  07/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_COREINT_TEST_H_
#define BBQUE_COREINT_TEST_H_

#include <memory>
#include <vector>
#include "bbque/object.h"
#include "bbque/system_view.h"
#include "bbque/app/application.h"
#include "bbque/plugins/plugin.h"
#include "bbque/plugins/test.h"

#ifndef BBQUE_DYNAMIC_PLUGIN
# define CoreInteractionsTest CoreInteractionsTestS
# define	PLUGIN_TYPE "STATIC"
#else
# define CoreInteractionsTest CoreInteractionsTestD
# define	PLUGIN_TYPE "DYNAMIC"
#endif

#define COREINT_NAMESPACE "coreint"

using namespace bbque::app;
using bbque::ApplicationManager;

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

/**
 * @class CoreInteractionsTest
 *
 * This test class simulate core interactions between applications and
 * Barbeque RTRM.
 *
 * By using the term "core" we are excluding the part regarding the
 * communication interface with the RTLib. Indeed the focus of the test is to
 * verify the correctness of the changes of scheduling status of applications.
 * Such changes are triggered by propèr method calls.
 *
 * As a consequence of scheduling changes, a variation in resource usages
 * accounting should be observed.
 *
 * Platform initialization is simulated too through a function that, given an
 * hard-coded set of resources, invokes the resource registration method of
 * ResourceAccounter for each of them.
 */
class CoreInteractionsTest: public TestIF, public Object {

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
	~CoreInteractionsTest();

	/**
	 * @brief Test launcher
	 */
	void Test();

private:

	/** System view instance */
	SystemView & sv;

	/** Application manager instance */
	ApplicationManager & am;

	/**
	 * @brief Constructor
	 */
	CoreInteractionsTest();

	/**
	 * @brief Test application reconfiguration action
	 *
	 * @param test_app A shared pointer to the application descriptor
	 * @param wm Working mode to switch in
	 * @param ov_time Switching time overhead
	 */
	void testScheduleSwitch(AppPtr_t & test_app, std::string const & wm,
			double ov_time);

	/**
	 * @brief Test working mode reconfigurations and constraints assertion
	 *
	 * @param am Application Manager instance
	 * @param test_app Object application test
	 */
	void testApplicationLifecycle(AppPtr_t & test_app);

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_COREINT_TEST_H_

