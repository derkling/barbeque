/**
 *       @file  test_test.cc
 *      @brief  Test the regression test system
 *
 * This provides a dummy example testcase for the regression test system.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  01/10/2012
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "tests.h"

// These are a set of useful debugging log formatters
#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "TEST       [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "TEST       [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "TEST       [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "TEST       [ERR]", fmt)

TestResult_t test_test(int argc, char *argv[]) {
	fprintf(stderr, FMT_INF("Here is a regression test's test\n"));
	if (argc < 1)
		return TEST_FAILED;

	fprintf(stderr, FMT_DBG("Test case [%s]\n"), argv[1]);

	switch (argv[1][0]) {
	case 'P':
		fprintf(stderr, FMT_INF("Here is a PASSED test example...\n"));
		return TEST_PASSED;

	case 'W':
		fprintf(stderr, FMT_WRN("Here is a WARNING test example...\n"));
		return TEST_WARNING;

	default:
		fprintf(stderr, FMT_ERR("Here is a FAILED test example...\n"));
		break;
	}

	return TEST_FAILED;
}
