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

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

/**
 * @class CoreInteractionsTest
 *
 * It embeds a set of calls for testing the managing of an application
 * lifecycle inside Barbeque RTRM. The class is the core of a plugin is used
 * for testing purpose.
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

	/**
	 * @brief Constructor
	 */
	CoreInteractionsTest();

	/**
	 * @brief Register some resources and print out info
	 */
	void RegisterSomeResources();

	/**
	 * @brief Print resource availabilities.
	 */
	void PrintResourceAvailabilities();

	/**
	 * @brief Print working modes details of an application
	 *
	 * @param test_app A shared pointer to the application descriptor
	 * @return 0 for succes, or other values for errors
	 */
	int WorkingModesDetails(
			std::shared_ptr<app::ApplicationStatusIF> test_app);

	/**
	 * @brief Print scheduling information of a given application
	 */
	void PrintScheduleInfo(std::shared_ptr<app::Application> test_app);

	/**
	 * @brief Get a single working mode and set next schedule.
	 * @param test_app A shared pointer to the application descriptor
	 * @param wm Working mode identifying name
	 * @param ov_time Switching time overhead
	 */
	void DoScheduleSwitch(std::shared_ptr<app::Application> test_app,
			std::string const & wm, double ov_time);

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_COREINT_TEST_H_

