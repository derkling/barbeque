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

#include "app.h"

#include <algorithm>
#include <cstring>
#include <cmath>
#include <random>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

/**
 * The RNG used for testcase initialization.
 */
std::mt19937 rng_engine(time(0));


// Public methods

RTLIB_ExitCode_t DemoApplication::Start() {
	return RTLIB_Init(DEMO_APP_NAME, &rtlib);
}


RTLIB_ExitCode_t DemoApplication::RegisterEXC() {
	char recipe_name[] = DEMO_RECIPE_NAME;

	RTLIB_ExecutionContextParams exc_params = {
		{RTLIB_VERSION_MAJOR, RTLIB_VERSION_MINOR},
		RTLIB_LANG_CPP,
		recipe_name
	};

	// "exc_hdl" here is as private member
	exc_hdl = rtlib->Register("hello_world", &exc_params);

	if (!exc_hdl)
		return RTLIB_EXC_NOT_REGISTERED;

	return RTLIB_OK;
}


RTLIB_ExitCode_t DemoApplication::EnableEXC() {
	return rtlib->Enable(exc_hdl);
}


RTLIB_ExitCode_t DemoApplication::DisableEXC() {
	return rtlib->Disable(exc_hdl);
}


void HelloWorldTask();


RTLIB_ExitCode_t DemoApplication::ControlLoop() {
	RTLIB_ExitCode_t result;
	RTLIB_WorkingModeParams wmp;
	// Number of threads for the task
	int num_threads;

	// Just a finite number of iteration in this example
	for (int i = 0; i < 5; ++i) {

		// 1. Require a working mode to the RTRM
		result = rtlib->GetWorkingMode(exc_hdl, &wmp, RTLIB_SYNC_STATELESS);

		// 2. Check and manage the RTRM response
		num_threads = CheckForReconfig(result, wmp);
		if (result != RTLIB_OK)
			continue;

		// 3. Run the task
		Run(HelloWorldTask, num_threads);

		// 4. QoS Monitoring
		QoSMonitor(wmp);

	}

	return result;
}


int DemoApplication::CheckForReconfig(RTLIB_ExitCode_t result,
		RTLIB_WorkingModeParams_t wmp) {

	static int num_thrds;

	switch (result) {
	case RTLIB_OK:
		// Continue to run in the same working mode
		break;

	case RTLIB_EXC_GWM_START:
	case RTLIB_EXC_GWM_RECONF:
	case RTLIB_EXC_GWM_MIGREC:
	case RTLIB_EXC_GWM_MIGRATE:
		// Reconfigure the task before running next step
		num_thrds = Configure(wmp);
		break;

	case RTLIB_EXC_GWM_BLOCKED:
		// Not scheduled. Don't execute.
		num_thrds = 0;
		break;

	default:
		cout << "Error: framework returned unexpected code" << endl;
		num_thrds = 0;
	}

	return num_thrds;
}


void DemoApplication::Run(void (*Task)(), int num_tds) {
	static uint8_t cycles = 0;
	fprintf(stderr, "\nRunning cycle [%d]...\n", ++cycles);

	for (int i = 0; i < num_tds; ++i) {
		thrds.push_back(thread(Task));
	}

	// Join threads
	JoinThreads(thrds);
	fprintf(stderr, "\n");
}


/// The task to execute
void HelloWorldTask() {
	fprintf(stderr, "Running: Hello World from thread %ld\n",
			syscall(SYS_gettid));
	sleep(2);
}


int DemoApplication::Configure(RTLIB_WorkingModeParams_t wmp) {
	cout << "Configure: Working mode = " << (uint16_t) wmp.awm_id << endl;

	switch (wmp.awm_id) {
	case 0:
		return 3;

	case 1:
		return 2;

	case 2:
		return 1;

	default:
		return 1;
	}
}


void DemoApplication::QoSMonitor(RTLIB_WorkingModeParams_t wmp) {
	if (wmp.awm_id > 2) {
		cout << "Set an higher value working mode" << endl;
	}
}


void jt(thread & t) {
	t.join();
}


void DemoApplication::JoinThreads(list<thread> & tdlist) {
	for_each(tdlist.begin(), tdlist.end(), jt);
	tdlist.clear();
}


void DemoApplication::UnregisterEXC() {
	rtlib->Unregister(exc_hdl);
}


RTLIB_ExitCode_t DemoApplication::Exit(RTLIB_ExecutionContextHandler_t ech,
		struct timespec timeout) {
	(void) timeout;
	(void) ech;

	return RTLIB_OK;
}

