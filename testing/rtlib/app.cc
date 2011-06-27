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

#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "BAPP       [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "BAPP       [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "BAPP       [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "BAPP       [ERR]", fmt)

BbqueApp::BbqueApp(std::string const & name) {
	(void)name;

	fprintf(stderr, FMT_INF("Initializing RPC library...\n"));

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
	excMap_t::iterator it = exc_map.find(name);

	fprintf(stderr, FMT_INF("Registering EXC [%s:exRecipe_%03d]...\n"),
			name.c_str(), recipe_id%999);

	if (it!=exc_map.end()) {
		fprintf(stderr, FMT_ERR("FAILED: EXC [%s:exRecipe_%03d] "
					"already registered...\n"),
				name.c_str(), recipe_id%999);
		return -1;
	}

	::snprintf(recipe_name, 13, "exRecipe_%03d", recipe_id%999);

	assert(rtlib && rtlib->RegisterExecutionContext);
	exc_hdl = rtlib->RegisterExecutionContext(name.c_str(), &exc_params);
	if (!exc_hdl) {
		fprintf(stderr, FMT_ERR("FAILED: registering EXC "
					"[%s:exRecipe_%03d]\n"),
				name.c_str(), recipe_id%999);
		return -2;
	}

	fprintf(stderr, FMT_INF("EXC [%s:exRecipe_%03d] registerd (@%p)...\n"),
			name.c_str(), recipe_id%999, (void*)exc_hdl);

	// Savingthe  EXC handler
	exc_map.insert(excMapEntry_t(name, exc_hdl));
	return 0;
}

void BbqueApp::UnregisterAllEXC() {
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	RTLIB_ExecutionContextHandler exc_hdl;

	fprintf(stderr, FMT_INF("Unregistering all EXC...\n"));

	for ( ; it!=end; ++it) {
		exc_hdl = (*it).second;
		assert(exc_hdl);
		fprintf(stderr, FMT_INF("Unregistering EXC [%s] (@%p)...\n"),
				(*it).first.c_str(), (void*)exc_hdl);

		assert(rtlib && rtlib->UnregisterExecutionContext);
		rtlib->UnregisterExecutionContext(exc_hdl);
		exc_map.erase(it);
	}

}

RTLIB_ExitCode BbqueApp::Start(uint8_t first, uint8_t last) {
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	RTLIB_ExecutionContextHandler exc_hdl;
	RTLIB_ExitCode result;

	fprintf(stderr, FMT_INF("Starting [%d] EXCs...\n"), last-first);

	for (uint8_t idx=0; it!=end; ++it) {
		// Starting only the selected EXCs
		if (idx<first)
			continue;
		if (idx>last)
			break;

		exc_hdl = (*it).second;
		assert(exc_hdl);
		fprintf(stderr, FMT_INF("Starting EXC [%s] (@%p)...\n"),
				(*it).first.c_str(), (void*)exc_hdl);

		assert(rtlib && rtlib->UnregisterExecutionContext);
		result = rtlib->StartExecutionContext(exc_hdl);
		if (result!=RTLIB_OK) {
			fprintf(stderr, FMT_INF("EXC [%s] (@%p) "
						"START FAILED\n"),
				(*it).first.c_str(), (void*)exc_hdl);
			return result;
		}
	}

	return RTLIB_OK;
}

int BbqueApp::GetWorkingMode(std::string const & name) {
	excMap_t::iterator it = exc_map.find(name);
	RTLIB_WorkingModeParams wmp;
	RTLIB_ExitCode result;

	fprintf(stderr, FMT_INF("Get AWM for EXC [%s]...\n"),
			name.c_str());

	if (it==exc_map.end()) {
		fprintf(stderr, FMT_ERR("FAILED: EXC [%s] not registered\n"),
				name.c_str());
		return -1;
	}

	assert(rtlib && rtlib->GetWorkingMode);

	RTLIB_ExecutionContextHandler ech = (*it).second;
	result = rtlib->GetWorkingMode(ech, &wmp);
	if (result != RTLIB_OK) {
		fprintf(stderr, FMT_ERR("FAILED: get AWM for EXC [%s]\n"),
				name.c_str());
		return -2;
	}

	fprintf(stderr, FMT_INF("EXC [%s] assigned AWM [%d]\n"),
			name.c_str(), wmp.awm_id);

	return 0;
}

RTLIB_ExitCode BbqueApp::Stop(uint8_t first, uint8_t last) {
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	RTLIB_ExecutionContextHandler exc_hdl;
	RTLIB_ExitCode result;

	fprintf(stderr, FMT_INF("Stopping [%d] EXCs...\n"), last-first+1);

	for (uint8_t idx=0; it!=end; ++it) {
		// Starting only the selected EXCs
		if (idx<first)
			continue;
		if (idx>last)
			break;

		exc_hdl = (*it).second;
		assert(exc_hdl);
		fprintf(stderr, FMT_INF("Stopping EXC [%s] (@%p)...\n"),
				(*it).first.c_str(), (void*)exc_hdl);

		assert(rtlib && rtlib->UnregisterExecutionContext);
		result = rtlib->StopExecutionContext(exc_hdl);
		if (result!=RTLIB_OK) {
			fprintf(stderr, FMT_INF("EXC [%s] (@%p) STOP FAILED\n"),
				(*it).first.c_str(), (void*)exc_hdl);
			return result;
		}
	}

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueApp::Stop(
	RTLIB_ExecutionContextHandler ech,
	struct timespec timeout) {
	(void)ech;
	(void)timeout;

	return RTLIB_OK;
}



