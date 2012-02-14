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

#include "tests.h"

// These are a set of useful debugging log formatters
#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "TEST [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "TEST [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "TEST [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "TEST [ERR]", fmt)

/**
 * The RNG which could be used for testcase initialization.
 */
std::mt19937 rng_engine(time(0));

/**
 * The simulation timer
 */
bbque::utils::Timer test_tmr;

/**
 * The services exported by the RTLib
 */
RTLIB_Services_t *rtlib;


#define RUN_TEST(NAME)\
	extern TestResult_t test_test(int argc, char *argv[]);\
	test_argv[1] = # NAME;\
	tcount++;\
	fprintf(stderr,\
			FMT_INF("=====[ START Test #%03d: %s]=======================\n"),\
			tcount, # NAME);\
	tresult = test_test(2, (char**)test_argv);\
	if (tresult == TEST_FAILED) {\
		fprintf(stderr,\
				FMT_ERR("=====[ END   Test #%03d: %s, FAILED]===============\n"),\
				tcount, # NAME);\
		fprintf(stderr,"\n\n");\
		return EXIT_FAILURE;\
	}\
	if (tresult == TEST_PASSED)\
		fprintf(stderr,\
				FMT_INF("=====[ END   Test #%03d: %s, SUCCESS]==============\n"),\
				tcount, # NAME);\
	else if (tresult == TEST_WARNING)\
		fprintf(stderr,\
				FMT_WRN("=====[ END   Test #%03d: %s, WARNING]==============\n"),\
				tcount, # NAME);\
	fprintf(stderr,"\n\n")

//int main(int argc, char *argv[]) {
int test_all(int argc, char *argv[]) {
	uint16_t tcount = 0;
	const char *test_argv[] = { "all", ""};
	TestResult_t tresult;
	(void)argc;
	(void)argv;

	fprintf(stderr, FMT_INF("\n\t\t.:: BabrequeRTRM Regression Tests ::.\n"));

	// Starting the simulation timer
	test_tmr.start();

	// Initializing the RTLib library
	fprintf(stderr, FMT_INF("Init RTLib library...\n"));

	RTLIB_Init("BbqTesting", &rtlib);
	assert(rtlib);

	fprintf(stderr,"\n\n");
	DB(
	RUN_TEST(Passed);
	RUN_TEST(Warning);
	fprintf(stderr, FMT_WRN("NOTE: The next test is expected to fails!\n\n"));
	RUN_TEST(Failed);
	)
	(void)test_argv; // quite compilation warning on RELEASE build
	(void)tresult;   // quite compilation warning on RELEASE build

	//===== START Script-generated tests
	
	// <SCRIPT_TOKEN> do NOT remove this line
	
	//===== END   Script-generated tests

	fprintf(stderr, FMT_INF("All %d BBQ Tests completed with SUCCESS, "
				"in %3.3f[s]\n\n"), tcount, test_tmr.getElapsedTime());
	return EXIT_SUCCESS;

}

