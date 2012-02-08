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
