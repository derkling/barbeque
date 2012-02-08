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

#ifndef BBQUE_OVERHEADS_H_
#define BBQUE_OVERHEADS_H_

#include <cstdint>

namespace bbque { namespace app {

/**
 * @brief Overheads occourring when switching from a working
 * mode to a destination one.
 *
 * When an application switches its working mode (due to Optimizer decision),
 * Barbeque RTRM keep track of the overheads occourred to make the transition.
 * Get this information is worthwhile. Indeed every transition decision should
 * weight the benefits of the choice against the costs of making it.
 *
 * This object should be stored in a map container in an application working
 * mode descriptor, where the map key is the name of the destination working
 * mode.
 */
class TransitionOverheads {

public:

	/**
	 * @brief Constructor
	 * @param time The reconfiguration time measured
	 */
	TransitionOverheads(double time):
		switch_count(1) {

		last_switch_time = time;
		max_switch_time = time;
		min_switch_time = time;
	}

	/**
	 * @brief
	 *
	 * Set the time estimated / measured during the application
	 * reconfiguration from its current working mode to the new one scheduled
	 *
	 * @param time The reconfiguration time measured
	 */
	inline void SetSwitchTime(double time) {
		// Set last switch time
		last_switch_time = time;

		// Update max and min values
		if ((min_switch_time == 0) && (max_switch_time == 0)) {
			min_switch_time = max_switch_time = time;
		}
		else if (min_switch_time > time) {
			min_switch_time = time;
		}
		else if (max_switch_time < time) {
			max_switch_time = time;
		}
	}

	/**
	 * @brief Get the transition count value
	 * @return Number of transitions
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
	 * @brief Transition time measured in the last reconfiguration process
	 * @return Switch time of the last transition
	 */
	inline double LastTime() const {
		return last_switch_time;
	}

	/**
	 * @brief Minimum time measured for this transition over all the
	 * reconfigurations
	 * @return Minimum time registered
	 */
	inline double MinTime() const {
		return min_switch_time;
	}

	/**
	 * @brief Maximum time measured for this transition over all the
	 * reconfigurations
	 * @return Maximum time registered
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
