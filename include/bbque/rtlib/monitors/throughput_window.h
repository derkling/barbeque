/**
 *      @file   throughput_window.h
 *      @class  ThroughputWindow
 *      @brief  Type of window used by throughput monitor
 *
 * This class provides a window specifically created for the time monitor.
 *
 *     @author  Andrea Di Gesare , andrea.digesare@gmail.com
 *     @author  Vincenzo Consales , vincenzo.consales@gmail.com
 *
 *   @internal
 *     Created
 *    Revision
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Di Gesare Andrea, Consales Vincenzo
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */


#ifndef BBQUE_THROUGHPUT_WINDOW_H_
#define BBQUE_THROUGHPUT_WINDOW_H_

#include "generic_window.h"

class ThroughputWindow : public GenericWindow <double> {
public:

	ThroughputWindow(std::vector<GenericWindow<double>::Target> targets):
				GenericWindow<double>(targets){}

	ThroughputWindow(std::vector<GenericWindow<double>::Target> targets,
			 uint16_t windowSize):
				GenericWindow<double>(targets, windowSize){}

	/**
	 * @brief Indicates whether a starting time has been set or not
	 */
	bool started;
	/**
	 * @brief The start time of the basic time monitor
	 */
	std::chrono::monotonic_clock::time_point tStart;
	/**
	 * @brief The stop time of the basic time monitor
	 */
	std::chrono::monotonic_clock::time_point tStop;
};
#endif /* BBQUE_THROUGHPUT_WINDOW_H_ */
