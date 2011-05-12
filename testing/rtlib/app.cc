/**
 *       @file  dummy_app.cc
 *      @brief  A dummy application which interact with the barbeque RTRM
 *
 * This is a really simple application which could be used as a template to
 * develop real applications interacting with the Barbeque RTRM
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
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

#include "app.h"

#include "utility.h"

#include <assert.h>

#define LOG(fmt, ...) LOGGER(COLOR_GREEN, "BAPP        ", fmt, ## __VA_ARGS__)

#ifndef DBG
# define DBG(fmt, ...) LOGGER(COLOR_LGRAY, "BAPP        ", fmt, ## __VA_ARGS__)
#endif

BbqueApp::BbqueApp(std::string const & name) {
	(void)name;

	LOG("Initializing RPC library...");

	rtlib = RTLIB_Init(name.c_str());
	assert(rtlib);

}

int BbqueApp::RegisterEXC(std::string const & name, uint8_t recipe_id) {
	char recipe_name[] = "exRecipe_000";
	RTLIB_ExecutionContextParams exc_params = {
		{RTLIB_VERSION_MAJOR, RTLIB_VERSION_MINOR},
		RTLIB_LANG_CPP,
		recipe_name,
		BbqueApp::Stop
	};
	RTLIB_ExecutionContextHandler exc_hdl;

	LOG("Registering EXC [%s:exRecipe_%03d]...", name.c_str(), recipe_id%999);

	::snprintf(recipe_name, 13, "exRecipe_%03d", recipe_id%999);

	assert(rtlib && rtlib->RegisterExecutionContext);
	exc_hdl = rtlib->RegisterExecutionContext(name.c_str(), &exc_params);
	exc_hdls.push_back(exc_hdl);

	return 0;
}

void BbqueApp::Start() {

	LOG("Execution Context START...");


}

RTLIB_ExitCode BbqueApp::Stop(RTLIB_ExecutionContextHandler ech,
		struct timespec timeout) {
	LOG("Execution Context STOP...");

	return RTLIB_OK;
}




