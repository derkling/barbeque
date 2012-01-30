/**
 *      @file   TimeMonitor.h
 *      @class  TimeMonitor
 *      @brief  Time monitor
 *
 * This class is a specialization of the general monitor class, it provides
 * tools to manage the time monitors.
 * In addition to the monitors with the complete gestion of the windows of old
 * values it also offers a basically monitor without any advance features.
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

#ifndef BBQUE_TIME_MONITOR_H_
#define BBQUE_TIME_MONITOR_H_

#include "monitor.h"
#include "time_window.h"
#include <chrono>
#include <mutex>

class TimeMonitor : public Monitor <uint32_t> {
public:

	/**
	 * @brief Creates a new monitor with a window containing an history of
	 * previous values
	 *
	 * @param goal Value of goal required
	 * @param windowSize Number of elements in window of values
	 */
	uint16_t newGoal(uint32_t goal, uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Creates a new monitor with a window containing an history of
	 * previous values
	 *
	 * @param goal Value of goal required
	 * @param fType Selects the DataFunction for the evaluation of goal
	 * @param cType Selects the ComparisonFunction for the evaluation of goal
	 * @param windowSize Number of elements in window of values
	 */
	uint16_t newGoal(DataFunction fType,
			 ComparisonFunction cType,
			 uint32_t goal,
			 uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Creates a new monitor with a window keeping track of old values
	 *
	 * @param targets List of targets for the current goal
	 * @param windowSize Number of elements in the window of values
	 */
	uint16_t newGoal(std::vector<TimeWindow::Target> targets,
			 uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Deletes all the values previously saved
	 *
	 * @param id Identifies monitor and corresponding list
	 */
	void resetGoal(uint16_t id);

	/**
	 * @brief Starts a new measure of throughput for the monitor associated
	 * to the id given as parameter
	 *
	 * @param id Identifies monitor and corresponding list
	 */
	void start(uint16_t id);

	/**
	 * @brief Stops previous measure and saves the result in the right
	 * window (given by the id)
	 *
	 * @param id Identifies monitor and corresponding list
	 */
	void stop(uint16_t id);


	/**
	 * \addtogroup basicMonitors
	 *
	 *
	 * @{
	 * @defgroup basicTime Basic Time monitor
	 * These functions are used for a timeMonitor without a window
	 * of values
	 *
	 * @{
	 */

	/**
	 * @brief Starts new basic time monitor
	 * @ingroup basicTime
	 */
	void start();

	/**
	 * @brief Stops basic time monitor
	 * @ingroup basicTime
	 */
	void stop();

	/**
	 * @brief Returns elapsed time for the basic time monitor (in seconds)
	 * @ingroup basicTime
	 */
	double getElapsedTime();

	/**
	 * @brief Returns elapsed time for the basic time monitor
	 * (in milliseconds)
	 * @ingroup basicTime
	 */
	double getElapsedTimeMs();

	/**
	 * @brief Returns elapsed time for the basic time monitor
	 * (in microsecond)
	 * @ingroup basicTime
	 */
	double getElapsedTimeUs();

	/**
	 * @} @}
	 */

private:

	/**
	 * @brief Mutex variable associated to the timer.
	 */
	std::mutex timerMutex;

	/**
	 * @brief Start time of the Basic Time Monitor
	 */
	std::chrono::monotonic_clock::time_point tStart;

	/**
	 * @brief Stop time of the Basic Time Monitor
	 */
	std::chrono::monotonic_clock::time_point tStop;

	/**
	 * @brief Auxiliary varible for Basic Time Monitor
	 */
	bool started;

	/**
	 * @brief Locked starts new basic time monitor
	 */
	void _start();

	/**
	 * @brief Locked stops basic time monitor
	 */
	void _stop();

};

#endif /* BBQUE_TIME_MONITOR_H_ */
