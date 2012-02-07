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

#include <cstdio>
#include <iostream>
#include <random>
#include <cstring>

#include "utility.h"
#include "bbque_exc.h"

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
 * @brief The EXC handler
 */
pBbqueEXC_t pexc;


void usage(const char *name) {
	std::cout << "Usage: " << name <<
		" <rcp> <st>\n"
		"Where:" << std::endl;
	std::cout << 
		"<rcp> - recipe name\n"
		"<st>  - simulation time [s]\n"
		"\n\n" << std::endl;
}

int main(int argc, char *argv[]) {
	uint16_t simulation_time;
	char *rcp_name;

	std::cout << "\n\t\t.:: Simple application to showcase the Barbque RTRM ::.\n"
		<< std::endl;

	// Dummy and dirty command line processing
	if (argc < 2 || !sscanf(argv[2], "%hu", &simulation_time)) {

		fprintf(stderr, FMT_ERR("Missing or wrong parameters\n"));

		usage(::basename(argv[0]));
		return EXIT_FAILURE;
	}
	rcp_name = argv[1];

	// Starting the simulation timer
	simulation_tmr.start();

	// Initializing the RTLib library and setup the communication channel with
	// the Barbeque RTRM
	fprintf(stderr, FMT_INF("STEP 0. Initializing RTLib library, "
				"application [%s]...\n"), ::basename(argv[0]));

	RTLIB_Init(::basename(argv[0]), &rtlib);
	assert(rtlib);

	fprintf(stderr, FMT_INF("STEP 1. Registering EXC, using Recipe [%s]...\n"),
			rcp_name);
	pexc = pBbqueEXC_t(new BbqueEXC(rcp_name, rcp_name, rtlib, false));
	assert(pexc);


	fprintf(stderr, FMT_INF("STEP 2. Enabling the EXC...\n"));
	pexc->Enable();


	fprintf(stderr, FMT_INF("STEP 3. Starting the EXC control threads...\n"));
	pexc->Start();


	fprintf(stderr, FMT_INF("STEP 4. Running control threads for %d[s]...\n"),
			simulation_time);
	::sleep(simulation_time);


	fprintf(stderr, FMT_INF("STEP 5. Releasing the EXC...\n"));
	pexc.reset();

	std::cout << "\n\n" << std::endl;
	return EXIT_SUCCESS;

}

