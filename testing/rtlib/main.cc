/**
 *       @file  main.cc
 *      @brief  A toy example application using the Barbque RTRM
 *
 * This provide a really simple (toy example) implementation for an application
 * accessing the Barbeque RTRM services.
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


#include <cstdio>
#include <iostream>
#include <random>

#include "utility.h"
#include "app.h"

#include <libgen.h>

#define LOG(fmt, ...) LOGGER(COLOR_BLUE, "MAIN        ", fmt, ## __VA_ARGS__)

#ifndef DBG
# define DBG(fmt, ...) LOGGER(COLOR_LGRAY, "MAIN        ", fmt, ## __VA_ARGS__)
#endif

/**
 * The RNG used for testcase initialization.
 */
std::mt19937 rng_engine(time(0));

/**
 * The simulation timer
 */
rtrm::Timer simulation_tmr;

int main(int argc, char *argv[]) {
	uint16_t simulation_time;
	uint8_t max_reconf_time;
	uint8_t num_exc;
	uint8_t max_pt;
	uint8_t max_rt;

	std::cout << "\n\t\t.:: Simple application using the Barbque RTRM ::.\n"
		<< std::endl;

	// Dummy and dirty command line processing
	if (argc<6						||
		!sscanf(argv[1], "%hu",  &simulation_time)	||
		!sscanf(argv[2], "%hu",  &max_reconf_time)	||
		!sscanf(argv[3], "%hhu", &num_exc)		||
		!sscanf(argv[4], "%hhu", &max_pt)		||
		!sscanf(argv[5], "%hhu", &max_rt) ) {

		std::cout << "Usage: " << ::basename(argv[0]) <<
			" <st> <rt> <ne> <mp> <mr>\n"
			"Where:" << std::endl;
		std::cout << 	"<st> - simulation time [s]\n"
				"<rt> - max reconfigurations interval time [s]\n"
				"<ne> - number of EXC to generate\n"
				"<mp> - max processing time [s] for each AWM\n"
				"<mr> - max reconfiguration time [s] for each"
						"AWM switch\n"
				"\n\n" << std::endl;
		return EXIT_FAILURE;
	}

	// Starting the simulation timer
	simulation_tmr.start();

	LOG("building application [%s]...", ::basename(argv[0]));
	BbqueApp app(::basename(argv[0]));

	LOG("starting application processing...");
	app.Start();

	LOG("running simulation for [%d]s", simulation_time);

	sleep(simulation_time);

	LOG("stopping application...");
	app.Stop();

	std::cout << "\n\n" << std::endl;

	return EXIT_SUCCESS;
}

