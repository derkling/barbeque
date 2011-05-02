/**
 *       @file  timer.h
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

#ifndef RTRM_TIMER_H_
#define RTRM_TIMER_H_

#include <time.h>
#include <unistd.h>
#include <mutex>

namespace rtrm {

class Timer {

public:

	/**
	 * @brief Build a new Timer object
	 */
	Timer();

	/**
	 * @brief Start the timer
	 * startCount will be set at this point.
	 */
	void start();

	/**
	 * @brief Stop the timer
	 * endCount will be set at this point.
	 */
	void stop();

	/**
	 * @brief get elapsed time in [s]
	 * @return [s] of elapsed time
	 */
	double getElapsedTime();

	/**
	 * @brief get elapsed time in [us]
	 * @return [us] of elapsed time
	 */
	double getElapsedTimeMs();

	/**
	 * @brief compute elapsed time in micro-second resolution.
	 * other getElapsedTime will call this first, then convert to
	 * correspond resolution.
	 */
	 double getElapsedTimeUs();

private:

	/**
	 * stop flag
	 */
	bool stopped;

	/**
	 * The mutex protecting timer concurrent access
	 */
	std::mutex timer_mtx;

	/**
	 * The timer start instant
	 */
	struct timespec start_ts;

	/**
	 * The timer stop instant
	 */
	struct timespec stop_ts;
};

} // namespace rtrm

#endif // RTRM_TIMER_H_

