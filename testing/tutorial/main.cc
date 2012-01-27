/**
 *       @file  main.cc
 *      @brief  A toy example application using the Barbque RTRM
 *
 * This provide a really simple (toy example) implementation for an application
 * accessing the Barbeque RTRM services.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  04/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */


#include <cstdio>
#include <random>

#include "app.h"
#include "utility.h"

// Tutorial messages file
std::ifstream msgs_file;

// Class for demo application
DemoApplication app;

// Function declarations
RTLIB_ExitCode_t StartApplication();
RTLIB_ExitCode_t RegisterExecutionContexts();
RTLIB_ExitCode_t EnableExecutionContexts();
void DoJob();


int main(int argc, char *argv[]) {
	RTLIB_ExitCode_t result;

	if (argc < 1) {
		std::cout << " Missing tutorial messages file." << std::endl
			<<"Usage: $./rtlib_demo tutorial-file-path" << std::endl;
		return -1;
	}

	clearScreen();
	msgs_file.open(argv[1]);

	result = StartApplication();
	if (result != RTLIB_OK) {
		std::cout << "RTLIB: Init failed." << std::endl;
		return -1;
	}

	result = RegisterExecutionContexts();
	if (result != RTLIB_OK) {
		std::cout << "RTLIB: Registration failed." << std::endl;
		return -2;
	}

	result = EnableExecutionContexts();
	if (result != RTLIB_OK) {
		std::cout << "RTLIB: Error in EXC enabling." << std::endl;
		return -3;
	}

	DoJob();

	msgs_file.close();
	return EXIT_SUCCESS;
}

RTLIB_ExitCode_t StartApplication() {
	TTR_MESSAGE("_SPLASH_BANNER", COLOR_LRED, false);
	TTR_MESSAGE("_SPLASH_MSG", COLOR_GRAY, true);

	TTR_MESSAGE("_INTRO_INCLUDE", COLOR_GRAY, false);
	TTR_MESSAGE("_PROG_INCLUDE", COLOR_WHITE, false);
	TTR_MESSAGE("_INTRO_INIT", COLOR_GRAY, false);
	TTR_MESSAGE("_PROG_INIT", COLOR_WHITE, false);
	TTR_MESSAGE("_OUTRO_INIT", COLOR_GRAY, true);
	return app.Start();
}

RTLIB_ExitCode_t RegisterExecutionContexts() {
	TTR_MESSAGE("_INTRO_EXC", COLOR_GRAY, false);
	TTR_MESSAGE("_PROG_REGEXC", COLOR_WHITE, false);
	TTR_MESSAGE("_OUTRO_REGEXC", COLOR_GRAY, true);
	return app.RegisterEXC();
}

RTLIB_ExitCode_t EnableExecutionContexts() {
	TTR_MESSAGE("_INTRO_ENABLE", COLOR_GRAY, false);
	TTR_MESSAGE("_PROG_ENABLE", COLOR_WHITE, true);
	return app.EnableEXC();
}

void DoJob() {

	TTR_MESSAGE("_INTRO_AWM", COLOR_GRAY, false);
	TTR_MESSAGE("_PROG_CLOOP", COLOR_WHITE, false);
	TTR_MESSAGE("_TEXT_EXAMP", COLOR_GRAY, true);
	// The application control loop
	app.ControlLoop();

	TTR_MESSAGE("_INFO_RECONF", COLOR_GRAY, true);
	TTR_MESSAGE("_INFO_EXIT", COLOR_GRAY, true);

	app.DisableEXC();
	app.UnregisterEXC();
	getchar();
	TTR_MESSAGE("_BYE_", COLOR_LRED, false);
}

