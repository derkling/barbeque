/**
*      @file   memory_monitor.h
*      @class  MemoryMonitor
*      @brief  Memory monitor
*
* This class provides a memory monitor
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

#ifndef BBQUE_MEMORY_MONITOR_H_
#define BBQUE_MEMORY_MONITOR_H_

#include "monitor.h"

#include <unistd.h>

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

	// * @ingroup basicMonitors
	// * @defgroup basicMonitors Basic Monitor
	/** @defgroup basicMonitors Basic Monitor
	*
	*  These functions are used for simple monitors (without a window of
	*  values)
	*/
	//@{
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

	//@}
};

#endif /* BBQUE_MEMORY_MONITOR_H_ */
