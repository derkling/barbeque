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
 * =============================================================================
 */

#ifndef BBQUE_UTILITY_H_
#define BBQUE_UTILITY_H_

#include "bbque/config.h"

#include <cstdio>
#include <sys/syscall.h>

#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <string>
#include <unistd.h>

#include "bbque/utils/timer.h"
#include "bbque/res/resources.h"

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


/** Type for ID used in resource path */
typedef int16_t ResID_t;

/** Return the PID of the calling process/thread */
inline pid_t gettid() {
	return syscall(SYS_gettid);
}

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
inline uint64_t ConvertValue(uint64_t value, std::string const & units) {

	if (units.empty())
		return value;

	switch(toupper(units[0])) {
	case 'K':
		return value *= POW_2_10;
	case 'M':
		return value *= POW_2_20;
	case 'G':
		return value *= POW_2_30;
	default:
		return value;
	}
}

/**
 * @brief Extract the head of a resource path.
 *
 * Split the resource path string in a "head" and a "tail", considering a
 * pattern wherein the separator char should be found. The head is returned,
 * while the tail is saved in the string object argument.
 *
 * The function is especially used to get the head of a path (the first
 * level/namespace).For instance, if the resource path is
 * "arch.clusters.mem0", the function returns "arch" and set <i>next_path</i>
 * to "clusters.mem0".
 *
 * Moreover it is used for path template construction too.
 *
 * @param tail The resource path to split
 * @param pattern The pattern of the separator char
 * @return The head of the path, and set <i>next_path</> with the tail path
 * left.
 */
inline std::string SplitAndPop(std::string & tail,
		const char * pattern = ".") {
	// Head of the path to return
	std::string _head;

	// Find the position of a  "_pattern" char in "tail"
	size_t dot_pos = tail.find_first_of(pattern);

	if (dot_pos != std::string::npos) {
		// Split head and tail of the path
		_head = tail.substr(0, dot_pos);
		tail = tail.substr(dot_pos + 1);
	}
	else {
		// Head == path (i.e "mem0", "dma0", ...)
		_head = tail;
		tail.clear();
	}
	// The head
	return _head;
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
	std::string _templ_path;
	std::string _tail = path;

	// Split the path using numbers as separator pattern and append the head
	// into the the path template
	do {
		_templ_path += SplitAndPop(_tail, "0123456789");
	} while (!_tail.empty());

	// The template path built
	return _templ_path;
}

/**
 * @brief Check if a path string is a template
 * @param path The path to check
 * @return True if it is, false otherwise
 */
inline bool IsPathTemplate(std::string const & path) {
	return (path.find_first_of("0123456789") == std::string::npos);
}


/**
 * @brief Append a resource ID number to a string
 *
 * @param orig_name Original string
 * @param rid Resource ID number
 * @return The string updated
 */
inline std::string AppendID(std::string const & orig_name, ResID_t rid) {
	std::string ret_name(orig_name);

	// Check ID validity
	if (rid <= RSRC_ID_ANY)
		return orig_name;

	std::stringstream ss;
	ss << rid;
	ret_name += ss.str();
	return ret_name;
}


/**
 * @brief Replace the ID of a resource in a path
 *
 * If the given resource name is contained into the resource path, substitute
 * its ID with the one specified in dst_ID.
 *
 * @param curr_rsrc_path The resource path
 * @param rsrc_name Name of the resource
 * @param src_ID ID to replace
 * @param dst_ID New ID number
 * @return The resource path updated
 */
inline std::string ReplaceResourceID(std::string const & curr_rsrc_path,
				std::string	const & rsrc_name,
				ResID_t src_ID,
				ResID_t dst_ID) {

	// Search the resource name in the current path
	std::string bind_rsrc_path(curr_rsrc_path);
	std::string rsrc_name_orig(AppendID(rsrc_name, src_ID));
	size_t start_pos = bind_rsrc_path.find(rsrc_name_orig);
	if (start_pos == std::string::npos)
		return bind_rsrc_path;

	// Replace it with the dst_ID-based form
	size_t dot_pos = bind_rsrc_path.find(".", start_pos);
	std::string bind_rsrc_name(AppendID(rsrc_name, dst_ID));
	bind_rsrc_path.replace(start_pos, (dot_pos - start_pos), bind_rsrc_name);
	return bind_rsrc_path;
}


/**
 * @brief Get ID of a resource in a path
 *
 * @param rsrc_path Resource path
 * @param rsrc_name Resource name
 * @return The ID of the resource if it is part of the part. RSRC_ID_NONE
 * otherwise.
 */
inline ResID_t GetResourceID(std::string const & rsrc_path,
				std::string	const & rsrc_name) {
	// Find the ID of the resource in the path
	size_t start_pos = rsrc_path.find(rsrc_name);
	if (start_pos == std::string::npos)
		return RSRC_ID_NONE;

	// Extract and return the ID value
	size_t dot_pos = rsrc_path.find(".", start_pos);
	std::string id(rsrc_path.substr(start_pos + rsrc_name.length(), dot_pos));
	return atoi(id.c_str());
}


#endif // BBQUE_UTILITY_H_

