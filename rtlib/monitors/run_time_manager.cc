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

#include <cmath>

#include <bbque/monitors/run_time_manager.h>
#include <iostream>

using std::vector;

namespace bbque { namespace rtlib { namespace as {

void RunTimeManager::setGoals(GoalsList &goalsList) {
	this->goalsList = goalsList;
}

bool RunTimeManager::checkGoals(GoalInfoList &goalsInfo) {

	bool result = true;
	GoalInfoPtr goalInfo;

	goalsInfo.clear();
	goalsInfo.reserve(goalsList.size());

	GoalsList::const_iterator it;

	for (it = goalsList.begin(); it != goalsList.end(); ++it) {
		goalInfo = (*it)->fullCheckGoal();
		goalsInfo.push_back(goalInfo);
		result = result && goalInfo->isAchieved();
	}

	return result;
}

/*
 * The two macros below, check if an upper or lower bound is present in
 * the opFilters structure and updates it with the new contraint calculated by
 * the adjustConstraints function. When no bound is found, the macro puts a new
 * constraint that is > or < even if the goal has been declared with >= or <=
 * TODO Consider whether to add automatically a constraint when a new goal is
 * declared. The cons is that if the model of the application is not exact,
 * when the application starts, the AS-RTM might not find any feasible point.
 * It will know the ratio between the modeled performance and actual ones only
 * after the first goal-checking phase.
 * Another possible solution is to avoid checking non-resource constraints at
 * the first check. We will consider it as well and take a decision promptly.
 */
#define UPDATE_UPPER_BOUND()\
found = false;\
for (itc = opFilters.begin(); itc!=opFilters.end();++itc){\
	if (itc->name == metricName && \
		(itc->cFunction.target_type() ==\
			ComparisonFunctors::Less.target_type() || \
		itc->cFunction.target_type() ==\
			ComparisonFunctors::LessOrEqual.target_type())) {\
			itc->value = newConstraint; \
			found = true;\
	}\
}\
if (!found) \
	opFilters.push_back(OPFilter(metricName, ComparisonFunctors::Less, \
				      newConstraint));

#define UPDATE_LOWER_BOUND()\
found = false;\
for (itc = opFilters.begin(); itc!=opFilters.end();++itc) {\
	if (itc->name == metricName && \
		(itc->cFunction.target_type() ==\
			ComparisonFunctors::Greater.target_type() || \
		itc->cFunction.target_type() ==\
			ComparisonFunctors::GreaterOrEqual.target_type())) { \
			itc->value = newConstraint;\
			found = true;\
		}\
}\
if (!found)\
	opFilters.push_back(OPFilter(metricName, ComparisonFunctors::Greater, \
				      newConstraint));

void RunTimeManager::adjustConstraints(const OperatingPoint &currentOp,
					       const GoalInfoList &goalsInfo,
					       OPFilterList &opFilters,
					       float switchThreshold) {

	bool found;
	bool achieved;
	bool adjustAll;
	double targetGoal;
	double observedValue;
	double oldConstraint;
	double newConstraint;
	double relativeError;
	std::string metricName;

	GoalInfoList::const_iterator it;
	OPFilterList::iterator itc;

	for (it = goalsInfo.begin(); it != goalsInfo.end(); ++it) {
		metricName = (*it)->metricName;
		oldConstraint = currentOp.metrics.find(metricName)->second;

		/*
		 * If one target of the goal needs to be adjusted proportionally
		 * to the change of performance, then all the other targets need
		 * to be adjusted as well
		 */
		achieved = (*it)->isAchieved();
		relativeError = fabs((*it)->getMaxRelativeError());

		adjustAll = false;
		if (!achieved || (achieved &&  relativeError > switchThreshold))
			adjustAll = true;

		for (uint8_t i = 0; i < (*it)->targetGoals.size(); ++i) {
			achieved      = (*it)->achieved.at(i);
			targetGoal    = (*it)->targetGoals.at(i);
			observedValue = (*it)->observedValues.at(i);
			relativeError = (*it)->relativeErrors.at(i);

			newConstraint = targetGoal * oldConstraint / observedValue;

			/*
			 * Updates all the previous constraints of that metric
			 * with the same comparison function than the new one
			 */
			if (achieved &&
				(adjustAll || fabs(relativeError) > switchThreshold)) {
				if (relativeError > 0) {
					UPDATE_LOWER_BOUND();
				} else {
					UPDATE_UPPER_BOUND();
				}

			} else if (!achieved) {
				if (relativeError < 0) {
					UPDATE_LOWER_BOUND();
				} else {
					UPDATE_UPPER_BOUND();
				}
			}
		}
	}
#ifdef DEBUG
	for (itc = opFilters.begin(); itc!=opFilters.end();++itc){
		std::cout<<"\t\t\tName "<<itc->name
			 <<" Value= "<<itc->value<<std::endl;
	}
#endif
}

void RunTimeManager::getNapAndRelativeError(const GoalInfoList &goalsInfo,
		uint8_t &maxNap, float &maxRelativeError) {

	vector<uint8_t> maxNaps;
	vector<double> maxRelativeErrors;
	GoalInfoList::const_iterator it;

	maxNaps.reserve(goalsInfo.size());
	maxRelativeErrors.reserve(goalsInfo.size());


	for (it = goalsInfo.begin(); it != goalsInfo.end(); ++it) {
		maxNaps.push_back((*it)->getMaxNap());
		maxRelativeErrors.push_back((*it)->getMaxRelativeError());
	}


	double absRelError;
	maxRelativeError = 0;
	std::vector<double>::const_iterator maxRit;

	for (uint8_t i=0;i<maxRelativeErrors.size();++i) {
		absRelError = fabs(maxRelativeErrors.at(i));
		if (absRelError > maxRelativeError)
			maxRelativeError = absRelError;
	}

	maxNap = *max_element(maxNaps.begin(), maxNaps.end());

}

bool RunTimeManager::getNextOp(OperatingPoint& op,
				       OPFilterList &opFilters,
				       float switchThreshold) {
	uint8_t maxNap;
	bool opChanged;
	bool goalAchieved;
	float maxRelativeError;

	GoalInfoList goalsInfo;

	if (goalsList.empty())
		return false;

	opChanged = false;

	goalAchieved = checkGoals(goalsInfo);

	getNapAndRelativeError(goalsInfo, maxNap, maxRelativeError);

	if (goalAchieved && maxRelativeError < switchThreshold)
		return false;
	adjustConstraints(op, goalsInfo, opFilters, switchThreshold);

	/*
	 * We consider that we want an higher point as soon as the goal is not
	 * achieved, while we could wait a while to switch on a lower point when
	 * the goal is achieved
	 * TODO Here the call to SetGoalGap is done when a goal is not achieved
	 * and there are no other feasible Operating Points in opList. While
	 * this is correct when a goal is not achieved because of a lack of
	 * resources, in other cases that could be unuseful and misleading for
	 * Barbeque. Moreover, the call to SetGoalNap seems to be expensive in
	 * term of execution time. It's really important to avoid calling it
	 * when unnecessary
	 *
	 * E.g: Consider a goal with a lower bound and an upper bound on
	 * throughput. While it is correct to call SetGoalGap when the
	 * throughput is below the lower bound, calling it when we are over the
	 * upper bound (hence, the goal is not achieved) is useless.
	 */
	if (!goalAchieved) {
		opChanged = opManager.getNextOP(op, opFilters);
		//TODO make this decision parametric
		if (!opChanged && maxNap > 20)
			bbqexc->SetGoalGap(maxNap);
	}
	else if (maxRelativeError >= switchThreshold)
		opChanged = opManager.getNextOP(op, opFilters);

	return opChanged;
}

bool RunTimeManager::getNextOp(OperatingPoint& op,
				       OPFilterList &opFilters,
				       const GoalInfoList &goalsInfo,
				       float switchThreshold) {
	uint8_t maxNap;
	bool opChanged;
	float maxRelativeError;

	if (goalsList.empty())
		return false;

	opChanged = false;

	getNapAndRelativeError(goalsInfo, maxNap, maxRelativeError);
	maxRelativeError = fabs(maxRelativeError);

	if (maxNap == 0 && maxRelativeError < switchThreshold)
		return false;
	adjustConstraints(op, goalsInfo, opFilters, switchThreshold);

	/*
	 * We consider that we want an higher point as soon as the goal is not
	 * achieved, while we could wait a while to switch on a lower point when
	 * the goal is achieved
	 * TODO Here the call to SetGoalGap is done when a goal is not achieved
	 * and there are no other feasible Operating Points in opList. While
	 * this is correct when a goal is not achieved because of a lack of
	 * resources, in other cases that could be unuseful and misleading for
	 * Barbeque. Moreover, the call to SetGoalNap seems to be expensive in
	 * term of execution time. It's really important to avoid calling it
	 * when unnecessary
	 *
	 * E.g: Consider a goal with a lower bound and an upper bound on
	 * throughput. While it is correct to call SetGoalGap when the
	 * throughput is below the lower bound, calling it when we are over the
	 * upper bound (hence, the goal is not achieved) is useless.
	 */
	if (maxNap > 0) {
		opChanged = opManager.getNextOP(op, opFilters);
		//TODO make this decision parametric
		if (!opChanged && maxNap > 20)
			bbqexc->SetGoalGap(maxNap);
	}
	else if (maxRelativeError >= switchThreshold)
		opChanged = opManager.getNextOP(op, opFilters);

	return opChanged;
}

} // namespace as

} // namespace rtlib

} // namespace bbque
