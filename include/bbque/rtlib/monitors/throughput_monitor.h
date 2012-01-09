/**
 *      @file   throughput_monitor.h
 *      @class  ThroughputMonitor
 *      @brief  Throughput monitor
 *
 * This class is a specialization of the general monitor class, it provides
 * tools to manage throughput monitors.
 * In addition to the monitors with the complete handling of previous old
 * values, it also offers a basic monitor without any advanced feature.
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
#ifndef BBQUE_THROUGHPUT_MONITOR_H_
#define BBQUE_THROUGHPUT_MONITOR_H_

#include <chrono>
#include <mutex>

#include "monitor.h"
#include "throughput_window.h"

/**
 * @brief A THROUGHPUT monitor
 * @ingroup rtlib_sec04_mon_thgpt
 *
 * This class is a specialization of the general monitor class which provides
 * tools to manage throughput monitors.  In addition to the monitors with the
 * complete handling of previous old values, it also offers a basic monitor
 * without any advanced feature.
 */
class ThroughputMonitor : public Monitor <double> {
public:

	/**
	 * @brief Creates a new monitor with a window containing an history of
	 * previous values
	 *
	 * @param goal Value of goal required
	 * @param windowSize Number of elements in the window of values
	 */
	uint16_t newGoal(double goal, uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Creates a new monitor with a window containing an history of
	 * previous values
	 *
	 * @param goal Value of goal required
	 * @param fType Selects the DataFunction for the evaluation of goal
	 * @param cType Selects the ComparisionFunction for the evaluation of
	 * goal
	 * @param windowSize Number of elements in window of values
	 */
	uint16_t newGoal(DataFunction fType,
			 ComparisonFunction cType,
			 double goal,
			 uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Creates a new monitor with a window keeping track of previous
	 * values
	 *
	 * @param targets List of targets for the current goal
	 * @param windowSize Number of elements in the window of values
	 */
	uint16_t newGoal(ThroughputWindow::TargetsPtr targets,
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
	 * @param data Amount of data analyzed in that amount of time
	 */
	void stop(uint16_t id, double data);

	/**
	 * @brief Starts a new basic throughput monitor
	 *
	 */
	void start();

	/**
	 * @brief Returns the throughput of basic throughput monitor
	 *
	 * @param data Amount of data analyzed in that amount of time
	 */
	double getThroughput(double data);

private:

	/**
	 * @brief Mutex variable associated to the timer.
	 */
	std::mutex timerMutex;

	/**
	 * @brief Start time of the Basic Throughput Monitor
	 */
	std::chrono::monotonic_clock::time_point tStart;

	/**
	 * @brief Stop time of the Basic Throughput Monitor
	 */
	std::chrono::monotonic_clock::time_point tStop;

	/**
	 * @brief Auxiliary varible for the Basic Throughput Monitor
	 */
	bool started;

	/**
	 * @brief Locked starts a new basic throughput monitor
	 */
	void _start();

	/**
	 * @brief Returns the throughput of basic throughput monitor
	 *
	 * @param data Amount of data analyzed in that amount of time
	 */
	double _getThroughput(const double &data);

	/**
	 * @brief Starts a new basic throughput monitor (locked)
	 */
	void _start(uint16_t id);

	/**
	 * @brief Stops previous measure and saves the result in the right
	 * window (given by the id) (locked)
	 *
	 * @param id Identifies monitor and corresponding list
	 * @param data Amount of data analyzed in that amount of time
	 */
	void _stop(uint16_t id, const double &data);
};

/*******************************************************************************
 *    Doxygen Module Documentation
 ******************************************************************************/

/**
 * @defgroup rtlib_sec04_mon_thgpt Throughput Monitoring
 * @ingroup rtlib_sec04_mon
 *
 * ADD MORE DETAILS HERE (Throughput Monitoring)
 *
 */

#endif /* BBQUE_THROUGHPUT_MONITOR_H_ */
