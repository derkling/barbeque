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

#ifndef BBQUE_TIME_MONITOR_H_
#define BBQUE_TIME_MONITOR_H_

#include <bbque/monitors/monitor.h>
#include <bbque/monitors/time_window.h>
#include <chrono>
#include <memory>
#include <mutex>

/**
 * @brief A TIME monitor
 * @ingroup rtlib_sec04_mon_time
 *
 * @details
 * This class is a specialization of the general monitor class, it provides
 * tools to manage the time monitors.  In addition to the monitors with the
 * complete gestion of the windows of old values it also offers a basically
 * monitor without any advance features.
 */
class TimeMonitor : public Monitor <uint32_t> {
public:

	/**
	 * @brief Default constructor of the class
	 */
	TimeMonitor() :
		started(false) {
	}

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
	uint16_t newGoal(TimeWindow::TargetsPtr targets,
			 uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Creates a monitor (without goals) with a window keeping track
	 * of old values
	 *
	 * @param windowSize Number of elements in the window of values
	 */
	uint16_t newEmptyGoal(uint16_t windowSize = defaultWindowSize);

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
	 * @brief Starts new basic time monitor
	 */
	void start();

	/**
	 * @brief Stops basic time monitor
	 */
	void stop();

	/**
	 * @brief Returns elapsed time for the basic time monitor (in seconds)
	 */
	double getElapsedTime();

	/**
	 * @brief Returns elapsed time for the basic time monitor
	 * (in milliseconds)
	 */
	double getElapsedTimeMs();

	/**
	 * @brief Returns elapsed time for the basic time monitor
	 * (in microsecond)
	 */
	double getElapsedTimeUs();

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

	/**
	 * @brief Locked starts new basic time monitor
	 * @param id Identifies monitor and corresponding list
	 *
	 */
	void _start(uint16_t id);

	/**
	 * @brief Locked stops basic time monitor
	 * @param id Identifies monitor and corresponding list
	 *
	 */
	void _stop(uint16_t id);

};

/*******************************************************************************
 *    Doxygen Module Documentation
 ******************************************************************************/

/**
 * @defgroup rtlib_sec04_mon_time Time Monitoring
 * @ingroup rtlib_sec04_mon
 *
 * ADD MORE DETAILS HERE (Time Monitoring)
 *
 */

#endif /* BBQUE_TIME_MONITOR_H_ */
