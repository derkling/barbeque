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
