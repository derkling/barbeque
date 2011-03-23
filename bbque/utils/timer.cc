/**
 *       @file  timer.cpp
 *      @brief  High Resolution Timer support
 *
 * This class provides basic service to measure elapres time with microseconds
 * accuracy.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/21/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/utils/timer.h"

#include <cstdlib>

namespace bbque { namespace utils {

Timer::Timer(bool running) :
	stopped(true) {
		if (running)
			start();
}

void Timer::start() {
	std::lock_guard<std::mutex> lg(timer_mtx);

	stopped = false;
	clock_gettime(CLOCK_REALTIME, &start_ts);
}

void Timer::stop() {
	std::lock_guard<std::mutex> lg(timer_mtx);

	stopped = true;
	clock_gettime(CLOCK_REALTIME, &stop_ts);
}

double Timer::getElapsedTimeUs() {
	std::lock_guard<std::mutex> lg(timer_mtx);
	double start, stop;

	if(!stopped)
		clock_gettime(CLOCK_REALTIME, &stop_ts);

	start = (start_ts.tv_sec * 1000000.0) + (start_ts.tv_nsec / 1000.0);
	stop  = (stop_ts.tv_sec * 1000000.0)  + (stop_ts.tv_nsec / 1000.0);

	return stop-start;
}

double Timer::getElapsedTimeMs() {
    return getElapsedTimeUs()/1000.0;
}

double Timer::getElapsedTime() {
    return getElapsedTimeUs()/1000000.0;
}

} // namespace utils

} // namespace bbque

