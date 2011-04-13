/**
 *       @file  utility.h
 *      @brief  A set of utility functions
 *
 * This provide a set of utility functions and definitions common to all other
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

#ifndef BBQUE_UTILITY_H_
#define BBQUE_UTILITY_H_

#include "bbque/config.h"

#include <stdio.h>
#include <cstdint>
#include <string>
#include "bbque/utils/timer.h"

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

extern bbque::utils::Timer bbque_tmr;

# define BBQUE_FMT(color, module, fmt) \
	        color "[%11.6f] " module ": " fmt "\033[0m", \
			bbque_tmr.getElapsedTime()

#ifdef BBQUE_DEBUG
# define DB(x) x
#else
# define DB(x)
#endif

#define POW_2_10 0x400
#define POW_2_20 0x100000
#define POW_2_30 0x40000000

/**
 * @brief Convert to unity
 *
 * Return the correct value, based on the units specified.
 * (i.e value=4 units="Kb" returns 4096).
 * By now the function supports just Kilo, Mega, Giga units.
 *
 * @param value The value to convert
 * @param units Units string
 * @return The value converted
 */
inline uint64_t ConvertValue(ulong value, std::string const & units) {

	if (!units.empty()) {
		switch(toupper(units.at(0))) {
		case 'K':
			value *= POW_2_10;
			break;
		case 'M':
			value *= POW_2_20;
			break;
		case 'G':
			value *= POW_2_30;
			break;
		default:
			value *= 1;
		}
	}
	return value;
}


/**
 * @brief Extract the head of a resource path.
 *
 * Pop the first namespace in a resource path string, and set the remaining
 * trail in "_next_path".  For instance, if the resource path is
 * "arch.clusters.mem0", the function returns "arch" and set <i>next_path</i>
 * to "clusters.mem0".
 *
 * @param next_path The resource path
 * @param pattern The pattern of the separator char
 * @return The head of the path, and set <i>next_path</> with the tail path
 * left.
 */
inline std::string PopPathLevel(std::string & next_path,
		const char * pattern = ".") {

	// Path level string to return
	std::string _curr_ns;

	// Find the position of a char in "_pattern"
	int dot_pos = next_path.find_first_of(pattern);

	if (dot_pos == -1) {
		// No separator char found
		_curr_ns = next_path;
		next_path.clear();
	}
	else {
		// Split head and tail
		_curr_ns = next_path.substr(0, dot_pos);
		next_path = next_path.substr(dot_pos + 1);
	}
	return _curr_ns;
}


/**
 * @brief Template of a resource path.
 *
 * Given a resource path like "arch.clusters.cluster2.pe1" the function
 * returns the template "arch.clusters.cluster.pe".
 *
 * This is useful for checking resource existance without refer to a specific
 * resource object (with its id-based path). This way allow to refer to a set
 * of resources of the same "class" at the same hierarchical level in the
 * resource tree.
 *
 * @param path Resource complete path (ID-based)
 * @return The template path (without resource IDs)
 */
inline std::string const PathTemplate(std::string const & path) {

	// Template path to return
	std::string _str_templ;

	// Extract the first node in the resource path
	std::string _ns_path = path;
	std::string _curr_ns = PopPathLevel(_ns_path, "0123456789");

	// Iterate over each node in the path
	while (true) {

		if (_curr_ns.empty())
			break;
		// Append the current node name
		_str_templ += _curr_ns;

		// Next node
		_curr_ns = PopPathLevel(_ns_path, "0123456789");

		// If this is not the last namespace in the path append a "."
		if (!_curr_ns.empty())
			_str_templ += ".";
	}
	// The template path built
	return _str_templ;
}

#endif // BBQUE_UTILITY_H_

