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

#include "bbque/rtlib/monitors/application_rtrm.h"

void ApplicationRTRM::setGoals(GoalsList &goalsList) {
	this->goalsList = goalsList;
}

void ApplicationRTRM::increaseResources() {
	bool result;
	float maxNap;

	GoalsList::const_iterator it;
	std::vector<float> naps;
	std::vector<float> maxNaps;

	if (! opManager.highestOP())
		return;

	for (it = goalsList.begin(); it != goalsList.end(); ++it) {
		result = (*it)->checkGoal(naps);
		if (result)
			return;
		maxNap = *max_element(naps.begin(), naps.end());
		maxNaps.push_back(maxNap);
	}
	maxNap = *max_element(maxNaps.begin(), maxNaps.end());
	bbqexc->SetGoalGap(100*maxNap);
}
