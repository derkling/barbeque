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

#ifndef BBQUE_UTILITY_H_
#define BBQUE_UTILITY_H_

#include <assert.h>
#include <unistd.h>
#include <cstdio>
#include <cstdint>
#include <cstdlib>

#include <sys/prctl.h>
#include <sys/syscall.h>

#include <bbque/utils/timer.h>

#define COLOR_WHITE  "\033[1;37m"
#define COLOR_LGRAY  "\033[37m"
#define COLOR_GRAY   "\033[1;30m"
#define COLOR_BLACK  "\033[30m"
#define COLOR_RED    "\033[31m"
#define COLOR_LRED   "\033[1;31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_LGREEN "\033[1;32m"
#define COLOR_BROWN  "\033[33m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE   "\033[34m"
#define COLOR_LBLUE  "\033[1;34m"
#define COLOR_PURPLE "\033[35m"
#define COLOR_PINK   "\033[1;35m"
#define COLOR_CYAN   "\033[36m"
#define COLOR_LCYAN  "\033[1;36m"

# define BBQUE_FMT(color, module, fmt) \
	        "\033[0m[%05d - %11.6f] " module ": " color fmt "\033[0m", \
			gettid(),\
			bbque_tmr.getElapsedTime()
#define FMT_INFO(fmt) BBQUE_FMT(COLOR_GREEN, "  - INFO   main            ", fmt)

#ifdef BBQUE_DEBUG
# define DB(x) x
#else
# define DB(x)
#endif

/** Get the pointer to the containing structure */
#define container_of(ptr, type, member)\
	(type*)((char*)(ptr) - offsetof(type, member))

/** Get number of entries of the specified array */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

/** Get a string lenght at compile time (null terminator not counted) */
#define STRLEN(s) ((sizeof(s)/sizeof(s[0]))-1)

/** Stringify the result of expansion of a macro argument
 * This requires to use two levels of macros.*/
#define STR(s) XSTR(s)
#define XSTR(s) #s

/** Optimize branch prediction for "taken" */
#define likely(x)       __builtin_expect((x),1)
/** Optimize branch prediction for "untaken" */
#define unlikely(x)     __builtin_expect((x),0)

/** Return the PID of the calling process/thread */
inline pid_t gettid() {
	return syscall(SYS_gettid);
}

#define BBQUE_MODULE_NAME(STR) "bq." STR

/** The High-Resolution timer exported by either the Barbeque and the RTLib */
extern bbque::utils::Timer bbque_tmr;

#endif // BBQUE_UTILITY_H_
