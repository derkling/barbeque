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

#include <bbque/monitors/application_rtrm.h>

void ApplicationRTRM::setGoals(GoalsList &goalsList) {
	this->goalsList = goalsList;
}
/*
 * The two macros below, check if an upper or lower bound is present in
 * the opFilters structure and updates it with the new contraint calculated by
 * the adjustConstraints function. When no bound is found, the macro puts a new
 * constraint that is > or < even if the goal has been declared with >= or <=
 * TODO Consider whether to add automatically a constraint when a new goal is
 * declared. If so, the fix above is not needed anymore because the contraints
 * will be always present in opFilters. The cons is that if the model of the
 * application is not exact (even just a proportional) when the application
 * starts, the AS-RTRM might not find any feasible point. It will know the ratio
 * between the modeled performance and actual ones only after the first
 * goal-checking phase. Another possible solution is to avoid checking
 * non-resource constraints at the first check. We will consider it as well and
 * take a decision soon.
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
	opFilters.push_back(OP_Filter(metricName, ComparisonFunctors::Less, \
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
	opFilters.push_back(OP_Filter(metricName, ComparisonFunctors::Greater, \
				      newConstraint));
