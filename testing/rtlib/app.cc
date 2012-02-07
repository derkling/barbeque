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

#include "utility.h"

#include <assert.h>

#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "BAPP       [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "BAPP       [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "BAPP       [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "BAPP       [ERR]", fmt)

BbqueApp::BbqueApp(std::string const & name) {
	(void)name;

	fprintf(stderr, FMT_INF("Initializing RPC library...\n"));

	RTLIB_Init(name.c_str(), &rtlib);
	assert(rtlib);

}

int BbqueApp::RegisterEXC(std::string const & name, uint8_t recipe_id) {
	char recipe_name[] = "exRecipe_000";
	RTLIB_ExecutionContextParams_t exc_params = {
		{RTLIB_VERSION_MAJOR, RTLIB_VERSION_MINOR},
		RTLIB_LANG_CPP,
		recipe_name,
		BbqueApp::Stop
	};
	RTLIB_ExecutionContextHandler_t exc_hdl;
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

	assert(rtlib && rtlib->Register);
	exc_hdl = rtlib->Register(name.c_str(), &exc_params);
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

void BbqueApp::UnregisterAll() {
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	RTLIB_ExecutionContextHandler_t exc_hdl;

	fprintf(stderr, FMT_INF("Unregistering all EXC...\n"));

	for ( ; it!=end; ++it) {
		exc_hdl = (*it).second;
		assert(exc_hdl);
		fprintf(stderr, FMT_INF("Unregistering EXC [%s] (@%p)...\n"),
				(*it).first.c_str(), (void*)exc_hdl);

		assert(rtlib && rtlib->Unregister);
		rtlib->Unregister(exc_hdl);
		exc_map.erase(it);
	}

}

RTLIB_ExitCode_t BbqueApp::Enable(uint8_t first, uint8_t last) {
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	RTLIB_ExecutionContextHandler_t exc_hdl;
	RTLIB_ExitCode_t result;

	fprintf(stderr, FMT_INF("Enabling [%d] EXCs...\n"), last-first);

	for (uint8_t idx=0; it!=end; ++it) {
		// Starting only the selected EXCs
		if (idx<first)
			continue;
		if (idx>last)
			break;

		exc_hdl = (*it).second;
		assert(exc_hdl);
		fprintf(stderr, FMT_INF("Enable EXC [%s] (@%p)...\n"),
				(*it).first.c_str(), (void*)exc_hdl);

		assert(rtlib && rtlib->Enable);
		result = rtlib->Enable(exc_hdl);
		if (result!=RTLIB_OK) {
			fprintf(stderr, FMT_INF("Enabling EXC [%s] (@%p) "
						"FAILED\n"),
				(*it).first.c_str(), (void*)exc_hdl);
			return result;
		}
	}

	return RTLIB_OK;
}

void BbqueApp::SwitchConfiguration(std::string const & name,
		RTLIB_WorkingModeParams_t & wmp) {
	fprintf(stderr, FMT_INF("Switching to new assigned AWM [%d] "
				"for EXC [%s] START\n"),
			wmp.awm_id, name.c_str());
	::usleep(100000);
	fprintf(stderr, FMT_INF("Switching to new assigned AWM [%d] "
				"for EXC [%s] END\n"),
			wmp.awm_id, name.c_str());

}

void BbqueApp::BlockExecution(std::string const & name) {
	fprintf(stderr, FMT_INF("Bloked execution for EXC [%s]\n"),
			name.c_str());

}

RTLIB_ExitCode_t BbqueApp::CheckForReconfiguration(std::string const & name,
		RTLIB_ExitCode_t result,
		RTLIB_WorkingModeParams_t & wmp) {

	switch (result) {
	case RTLIB_OK:
		fprintf(stderr, FMT_INF("Continue to run on the assigned AWM [%d] "
					"for EXC [%s]\n"),
				wmp.awm_id, name.c_str());
		return result;

	case RTLIB_EXC_GWM_START:
	case RTLIB_EXC_GWM_RECONF:
	case RTLIB_EXC_GWM_MIGREC:
	case RTLIB_EXC_GWM_MIGRATE:
		SwitchConfiguration(name, wmp);
		return result;

	case RTLIB_EXC_GWM_BLOCKED:
		BlockExecution(name);
		return result;

	default:
		DB(fprintf(stderr, FMT_ERR("Execution context [%s] GWM FAILED "
						"(Error: Invalid event [%d])\n"),
					prec->name.c_str(), result));
		assert(result >= RTLIB_EXC_GWM_START);
		assert(result <= RTLIB_EXC_GWM_BLOCKED);
	}

	return RTLIB_EXC_GWM_FAILED;
}

int BbqueApp::GetWorkingMode(std::string const & name) {
	excMap_t::iterator it = exc_map.find(name);
	RTLIB_WorkingModeParams_t wmp;
	RTLIB_ExitCode_t result;

	fprintf(stderr, FMT_INF("Get AWM for EXC [%s]...\n"),
			name.c_str());

	if (it == exc_map.end()) {
		fprintf(stderr, FMT_ERR("FAILED: EXC [%s] not registered\n"),
				name.c_str());
		return -1;
	}

	assert(rtlib && rtlib->GetWorkingMode);

	RTLIB_ExecutionContextHandler_t ech = (*it).second;

	// Looping until a valid AWM has been assinged
	do {
		result = rtlib->GetWorkingMode(ech, &wmp, RTLIB_SYNC_STATELESS);
		result = CheckForReconfiguration(name, result, wmp);
	} while ((result != RTLIB_OK) &&
			(result != RTLIB_EXC_GWM_FAILED));

	return result;
}

RTLIB_ExitCode_t BbqueApp::Disable(uint8_t first, uint8_t last) {
	excMap_t::iterator it(exc_map.begin());
	excMap_t::iterator end(exc_map.end());
	RTLIB_ExecutionContextHandler_t exc_hdl;
	RTLIB_ExitCode_t result;

	fprintf(stderr, FMT_INF("Disabling [%d] EXCs...\n"), last-first+1);

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

		assert(rtlib && rtlib->Disable);
		result = rtlib->Disable(exc_hdl);
		if (result!=RTLIB_OK) {
			fprintf(stderr, FMT_INF("EXC [%s] (@%p) STOP FAILED\n"),
				(*it).first.c_str(), (void*)exc_hdl);
			return result;
		}
	}

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueApp::Stop(
	RTLIB_ExecutionContextHandler_t ech,
	struct timespec timeout) {
	(void)ech;
	(void)timeout;

	return RTLIB_OK;
}

