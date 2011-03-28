/**
 *       @file  overheads.h
 *      @brief  Overheads in working mode transitions
 *
 * The class manages information upon the overheads occourring when applications
 * switch from a working mode to a another one (chosed by the optimizer).
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  01/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_OVERHEADS_H_
#define BBQUE_OVERHEADS_H_

#include <cstdint>
#include <bbque/object.h>

namespace bbque { namespace app {

/**
 * @class TransitionOverheads
 *
 * @brief Keep trace of the overheads occourring when switching from a working
 * mode to a destination one. This object should be stored in a map container
 * in an application working mode descriptor, where the map key is the name of
 * the destination working mode.
 */
class TransitionOverheads : public Object {

public:

	/**
	 * @brief Constructor
	 * @param time The reconfiguration time measured
	 */
	TransitionOverheads(double time):
		bbque::Object("TOverhead"),
		switch_count(1) {

		last_switch_time = time;
		max_switch_time = time;
		min_switch_time = time;
	}

	/**
	 * @brief Set the time estimated / measured during the application
	 * reconfiguration from its current working mode to the new one scheduled
	 * @param time The reconfiguration time measured
	 */
	void SetSwitchTime(double time);

	/**
	 * @brief Get the transition count value
	 */
	inline uint16_t Count() const {
		return switch_count;
	}

	/**
	 * @brief Increment the transition counter
	 */
	inline void IncCount() {
		++switch_count;
	}

	/**
	 * @brief Reset the transition counter
	 */
	inline void ResetCount() {
		switch_count = 0;
	}

	/**
	 * @brief The transition time measured in the last reconfiguration process
	 */
	inline double LastTime() const {
		return last_switch_time;
	}

	/**
	 * @brief The minimum time measured for this transition over all the
	 * 	reconfigurations
	 */
	inline double MinTime() const {
		return min_switch_time;
	}

	/**
	 * @brief The maximum time measured for this transition over all the
	 * 	reconfigurations
	 */
	inline double MaxTime() const {
		return max_switch_time;
	}

private:

	/** The minimum time spent in the transition process */
	double min_switch_time;

	/** The maximum time spent in the transition process */
	double max_switch_time;

	/** The time spent in transition from last to current working mode */
	double last_switch_time;

	/** A counter of switches */
	uint16_t switch_count;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_OVERHEADS_H_

