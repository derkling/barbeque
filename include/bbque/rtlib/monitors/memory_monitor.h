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

#include "monitor.h"

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
	 * @param goal Value of goal required
	 */
	uint16_t newGoal(uint32_t goal);

	/**
	 * @brief Creates a new monitor with a window containing an history of
	 * previous values
	 *
	 * @param goal Value of goal required
	 * @param windowSize Number of elements in the window of values
	 */
	uint16_t newGoal(uint32_t goal, uint16_t windowSize);

	/** @defgroup basicMemory Basic Memory monitor
	 *  These functions are for memoryMonitor without a window of values
	 */

	/**
	 * @brief Extracts current memory usage for the basic monitor
	 * @ingroup basicMemory
	 *
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

};

/*******************************************************************************
 *    Doxygen Module Documentation
 ******************************************************************************/

/**
 * @defgroup rtlib_sec04_mon_memory Memory Monitoring
 * @ingroup rtlib_sec04_mon
 *
 * ADD MORE DETAILS HERE (Memory Monitoring)
 *
 */

#endif /* BBQUE_MEMORY_MONITOR_H_ */
