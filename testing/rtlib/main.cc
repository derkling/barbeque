/**
 *       @file  rtlib/main.cc
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
#include <iostream>
#include <random>
#include <cstring>

#include "utility.h"
#include "bbque_exc.h"

#include <libgen.h>

#define EXC_BASENAME "exc"
#define RCP_BASENAME "exRecipe"

// These are a set of useful debugging log formatters
#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "MAIN       [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "MAIN       [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "MAIN       [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "MAIN       [ERR]", fmt)


/**
 * @brief A pointer to an EXC
 */
typedef std::shared_ptr<BbqueEXC> pBbqueEXC_t;

/**
 * @brief An entry of the map collecting managed EXCs
 */
typedef std::pair<std::string, pBbqueEXC_t> excMapEntry_t;

/**
 * @brief Maps recipes on corresponding EXCs
 */
typedef std::map<std::string, pBbqueEXC_t> excMap_t;

/**
 * The RNG used for testcase initialization.
 */
std::mt19937 rng_engine(time(0));

/**
 * The simulation timer
 */
rtrm::Timer simulation_tmr;

/**
 * The services exported by the RTLib
 */
RTLIB_Services_t *rtlib;

/**
 * @brief The map of managed EXCs
 */
excMap_t exc_map;

/**
 * @brief Register the required EXCs and enable them
 *
 * @param num_exc the number of EXCs to register
 */
RTLIB_ExitCode_t SetupEXCs(uint8_t num_exc) {
	char rcp_name[] = RCP_BASENAME "_00";
	char exc_name[] = EXC_BASENAME "_00";
	excMap_t::iterator it;
	pBbqueEXC_t pexc;

	fprintf(stderr, FMT_INF("STEP 1. Registering [%03d] EXCs...\n"),
			num_exc);

	for (uint8_t i = 0; i < num_exc; ++i) {

		// Setup EXC name and recipe name
		::snprintf(exc_name, ::strlen(exc_name)+1, EXC_BASENAME "_%02d", i);
		::snprintf(rcp_name, ::strlen(rcp_name)+1, RCP_BASENAME "_%02d", i);

		// Build a new EXC (without enabling it yet)
		assert(rtlib);
		pexc = pBbqueEXC_t(new BbqueEXC(exc_name, rcp_name, rtlib, false));

		// Saving the EXC (if registration to BBQ was successfull)
		if (pexc->isRegistered())
			exc_map.insert(excMapEntry_t(exc_name, pexc));

	}

	fprintf(stderr, FMT_INF("STEP 2. Enabling [%03u] registered EXCs...\n"),
			(unsigned)exc_map.size());

	it = exc_map.begin();
	for ( ; it != exc_map.end(); ++it) {
		pexc = (*it).second;

		// Enabling the EXC for scheduling
		assert(pexc);
		pexc->Enable();
	}


	fprintf(stderr, FMT_INF("STEP 3. Starting [%03u] EXCs control threads...\n"),
			(unsigned)exc_map.size());

	it = exc_map.begin();
	for ( ; it != exc_map.end(); ++it) {
		pexc = (*it).second;

		// Starting the control thread for the specified EXC
		assert(pexc);
		pexc->Start();
	}

	return RTLIB_OK;
}

/**
 * @brief Unregister all the EXCs
 */
RTLIB_ExitCode_t DestroyEXCs() {
	excMap_t::iterator it;

	fprintf(stderr, FMT_INF("Disabling [%03u] EXCs...\n"),
			(unsigned)exc_map.size());

	it = exc_map.begin();
	for ( ; it != exc_map.end(); ++it) {
		exc_map.erase(it);
	}

	return RTLIB_OK;
}

void usage(const char *name) {
	std::cout << "Usage: " << name <<
		" <ne> <mp> <mr> <rt> <st>\n"
		"Where:" << std::endl;
	std::cout << 
		"<ne> - number of EXC to register (max 99)\n"
		"<mp> - max processing time [s] for each AWM\n"
		"<mr> - max reconfiguration time [s] for each"
		"AWM switch\n"
		"<rt> - max reconfigurations interval time [s]\n"
		"<st> - simulation time [s]\n"
		"\n\n" << std::endl;
}

int main(int argc, char *argv[]) {
	uint16_t simulation_time;
	unsigned short max_reconf_time;
	unsigned short num_exc;
	unsigned short max_pt;
	unsigned short max_rt;

	std::cout << "\n\t\t.:: Simple application to showcase the Barbque RTRM ::.\n"
		<< std::endl;

	// Dummy and dirty command line processing
	if (argc < 6										||
			!sscanf(argv[1], "%hu", &num_exc)			||
			!sscanf(argv[2], "%hu", &max_pt)			||
			!sscanf(argv[3], "%hu", &max_rt)			||
			!sscanf(argv[4], "%hu", &max_reconf_time)	||
			!sscanf(argv[5], "%hu", &simulation_time)
			) {

		fprintf(stderr, FMT_ERR("Missing or wrong parameters\n"));

		usage(::basename(argv[0]));
		return EXIT_FAILURE;
	}

	// Upper limit the number of execution context to register
	if (num_exc > 99) {
		fprintf(stderr, FMT_ERR("Wrong parametes, EXC number must be <100\n"));

		usage(::basename(argv[0]));
		return EXIT_FAILURE;
	}

	// Starting the simulation timer
	simulation_tmr.start();

	// Initializing the RTLib library and setup the communication channel with
	// the Barbeque RTRM
	fprintf(stderr, FMT_INF("STEP 0. Initializing RTLib library, "
				"application [%s]...\n"), ::basename(argv[0]));

	RTLIB_Init(::basename(argv[0]), &rtlib);
	assert(rtlib);

	// Configuring required Execution Contexts
	SetupEXCs(num_exc);


	fprintf(stderr, FMT_INF("STEP 4. Running control threads for %d[s]...\n"),
			simulation_time);
	::sleep(simulation_time);

	// Releasing all the EXCs
	DestroyEXCs();


	std::cout << "\n\n" << std::endl;
	return EXIT_SUCCESS;

}

