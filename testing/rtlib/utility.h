/**
 *       @file  utility.h
 *      @brief  A set of utility functions
 *
 * This porvide a set of utility functions and definitions common to all other
 * modules.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/17/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef RTRM_UTILITY_H_
#define RTRM_UTILITY_H_

#include <stdio.h>
#include "timer.h"

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

extern rtrm::Timer simulation_tmr;

# define LOGGER(color, module, fmt, ...) \
	        printf(color "[%11.6f] " module " " fmt "\033[0m\n", \
			simulation_tmr.getElapsedTime(), ## __VA_ARGS__)

#ifdef RTRM_DEBUG
# define DBG(fmt, ...) do {} while(0);
#endif

#endif // RTRM_UTILITY_H_
