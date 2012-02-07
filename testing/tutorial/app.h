/**
 *       @file  tutorial/app.h
 *      @brief  
 *
 * Detailed description starts here.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  07/01/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/rtlib.h"

#include <fstream>
#include <iostream>
#include <list>
#include <vector>
#include <thread>

#define DEMO_RECIPE_NAME	"simple_1Tl2Cl2Pe"
#define DEMO_APP_NAME		"BBQDemoApplication"

using namespace std;

/**
 * @brief "Hello World" tutorial application
 *
 * This is a trivial tutorial application aiming to show how exploit Barbeque
 * RTRM framework.
 */
class DemoApplication {

public:

	RTLIB_ExitCode_t Start();

	RTLIB_ExitCode_t RegisterEXC();

	RTLIB_ExitCode_t EnableEXC();

	RTLIB_ExitCode_t DisableEXC();

	RTLIB_ExitCode_t ControlLoop();

	void UnregisterEXC();

	static RTLIB_ExitCode_t Exit(RTLIB_ExecutionContextHandler_t,
		struct timespec timeout);

private:

	/// Services of the RTRM framework
	RTLIB_Services_t * rtlib;

	/// The application will register just one EXC
	RTLIB_ExecutionContextHandler_t exc_hdl;

	/// Threads spawned for the task
	list<thread> thrds;

	/// Check the response of the Run-time resource manager.
	/// If the working mode is changed, a reconfiguration must be performed.
	int CheckForReconfig(RTLIB_ExitCode_t, RTLIB_WorkingModeParams_t);

	/// Run the task by spawning threads
	void Run(void (*task)(), int);

	/// Evaluate the QoS
	void QoSMonitor(RTLIB_WorkingModeParams_t wmp);

	/// Configure the number of thread to spawn accordingly to the working mode
	/// assigned and thus to the resources reserved by the RTRM
	int Configure(RTLIB_WorkingModeParams_t);

	/// Join all the threads in the list
	void JoinThreads(list<thread> &);
};
