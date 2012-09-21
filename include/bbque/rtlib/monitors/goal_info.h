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

#ifndef BBQUE_GOAL_INFO_H_
#define BBQUE_GOAL_INFO_H_

#include <memory>
#include <vector>
#include <algorithm>

#include <bbque/monitors/op_filter.h>

namespace bbque { namespace rtlib { namespace as {

typedef class GoalInfo {
public:

	/**
	 * Name of the metric associated to the goal
	 */
	std::string metricName;

	/**
	 * Goal-achievement of each target of the goal
	 */
	std::vector<bool> achieved;

	/**
	 * Targeted goal value of each target of the goal
	 */
	std::vector<double> targetGoals;

	/**
	 * Value observed by the monitoring phase of each target of the goal
	 */
	std::vector<double> observedValues;

	/**
	 * Relative error of each of the targets of the goal
	 */
	std::vector<double> relativeErrors;

	/**
	 * NAPs of each of the targets of the goal representing normalised
	 * values for the penalties regarding each target of the goal in the
	 * range [0,100]
	 */
	std::vector<uint8_t> naps;

	/**
	 * Default constructor of the class
	 */
	GoalInfo() {

	}

	/**
	 * @brief Constructor of the class that reserves nTargets places in the
	 * vectors included in the class
	 *
	 * @param nTargets number of elements expected (the number of targets
	 * of a goal)
	 */
	GoalInfo(uint8_t nTargets);

	/**
	 * @brief Returns if all the targets of the goal have been achieved or
	 * not
	 */
	bool isAchieved();

	/**
	 * @brief Returns the max NAP
	 */
	uint8_t getMaxNap();

	/**
	 * @brief Returns the relative error that has maximum absolute value
	 */
	float getMaxRelativeError();

} GoalInfo;

typedef std::shared_ptr<GoalInfo> GoalInfoPtr;
typedef std::vector<GoalInfoPtr> GoalInfoList;

inline GoalInfo::GoalInfo(uint8_t nTargets) {
	achieved.reserve(nTargets);
	relativeErrors.reserve(nTargets);
	naps.reserve(nTargets);
}

inline bool GoalInfo::isAchieved() {
	std::vector<bool>::iterator it;
	it = std::find(achieved.begin(), achieved.end(), false);
	if (it == achieved.end())
		return true;
	return false;
}

inline uint8_t GoalInfo::getMaxNap() {
	return *max_element(naps.begin(), naps.end());
}

inline float GoalInfo::getMaxRelativeError() {
	std::vector<double>::const_iterator it;
	double maxValue = 0;
	for (it=relativeErrors.begin();it!=relativeErrors.end();++it) {
		if (fabs(*it) > fabs(maxValue))
			maxValue = (*it);
	}
	return maxValue;
}

} // namespace as

} // namespace rtlib

} // namespace bbque

#endif /*BBQUE_GOAL_INFO_H_ */
