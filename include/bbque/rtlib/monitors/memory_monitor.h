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

#ifndef BBQUE_MEMORY_MONITOR_H_
#define BBQUE_MEMORY_MONITOR_H_

#include <bbque/monitors/monitor.h>

namespace bbque { namespace rtlib { namespace as {

/**
 * @brief A MEMORY monitor
 * @ingroup rtlib_sec04_mon_memory
 *
 * @details
 * This class provides a monitor on application memory usage.
 */
class MemoryMonitor: public Monitor <uint32_t> {
public:

	/**
	 * @brief Creates a new monitor with a window containing an history of
	 * previous values
	 *
	 * @param metricName Name of the metric associated to the goal
	 * @param goal Value of goal required
	 */
	uint16_t newGoal(std::string metricName, uint32_t goal);

	/**
	 * @brief Creates a new monitor with a window containing an history of
	 * previous values
	 *
	 * @param metricName Name of the metric associated to the goal
	 * @param goal Value of goal required
	 * @param windowSize Number of elements in the window of values
	 */
	uint16_t newGoal(std::string metricName, uint32_t goal,
			 uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Extracts current memory usage
	 *
	 * This function can be used to read the memory usage without using an
	 * associated goal and window of values.
	 */
	uint32_t extractMemoryUsage();

	/**
	 * @brief Extracts memory usage for the goal specified by the given id
	 *
	 * Adds a value in the window of the monitor in corrispondence with the
	 * correct id
	 *
	 * @param id Identifies monitor and corresponding list with old values
	 * of used memory
	 */
	uint32_t extractMemoryUsage(uint16_t id);

	/**
	 * @brief Extracts Virtual memory peak size
	 *
	 * This function can be used to get the maximum amount of virtual memory
	 * allocated for the current process.
	 */
	uint32_t extractVmPeakSize();

};

} // namespace as

} // namespace rtlib

} // namespace bbque

#endif /* BBQUE_MEMORY_MONITOR_H_ */
