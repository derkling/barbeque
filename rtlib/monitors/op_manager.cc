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

#include "bbque/rtlib/monitors/op_manager.h"

using namespace std;

bool OP_Manager::operatingPointsComparator::operator()(const OperatingPoint &op1,
						       const OperatingPoint &op2) const{
	double val1;
	double val2;
	std::string mName;
	for (unsigned int i=0; i < metricsPriorities.size(); ++i){
		mName = metricsPriorities[i].metricName;
		val1 = op1.metrics.find(mName)->second;
		val2 = op2.metrics.find(mName)->second;
		if (val1 == val2)
			continue;
		return metricsPriorities[i].comparisonFunction(val1, val2);
	}
	return false;
}

bool OP_Manager::getCurrentOP(OperatingPoint &op) {
	op = operatingPoints[vectorId];
	return true;
}

bool OP_Manager::getLowerOP(OperatingPoint &op) {
	if (vectorId >= (operatingPoints.size()-1))
		return false;

	vectorId++;
	op = operatingPoints[vectorId];
	isHighestOP = false;
	return true;
}

bool OP_Manager::getHigherOP(OperatingPoint &op) {
	if (vectorId == 0)
		return false;

	if(--vectorId == 0)
		isHighestOP = true;

	op = operatingPoints[vectorId];
	return true;
}

bool OP_Manager::isValidOP(const OperatingPoint &op,
			   const OP_FilterList &opFilters) const {
	bool result = true;
	OP_FilterList::const_iterator filter = opFilters.begin();;
	std::map<std::string, double>::const_iterator mappedValue;

	while (result && filter!=opFilters.end()) {
		mappedValue = op.metrics.find(filter->name);
		if (mappedValue == op.metrics.end())
			continue;
		result = (filter->cFunction(mappedValue->second, filter->value));
		++filter;
	}
	return result;
}


bool OP_Manager::getCurrentOP(OperatingPoint &op, const OP_FilterList &opFilters) {
	if (isValidOP(operatingPoints[vectorId], opFilters)){
		op = operatingPoints[vectorId];
		return true;
	}
	return getLowerOP(op,opFilters);
}

bool OP_Manager::getLowerOP(OperatingPoint &op, const OP_FilterList &opFilters) {
	for (unsigned int id = vectorId + 1;id < operatingPoints.size(); ++id) {
		if (isValidOP(operatingPoints[id],opFilters)){
			vectorId = id;
			op = operatingPoints[id];
			isHighestOP = false;
			return true;
		}
	}
	return false;
}

bool OP_Manager::getHigherOP(OperatingPoint &op, const OP_FilterList &opFilters) {
	for (int id = vectorId-1;id > 0; --id){
		if (isValidOP(operatingPoints[id], opFilters)){
			vectorId = id;
			op = operatingPoints[id];
			//Check routine for isHighestOP
			for(int id2 = id; id2 > 0; --id2){
				if (isValidOP(operatingPoints[id2], opFilters)){
					isHighestOP = false;
					return true;
				}
			}
			isHighestOP = true;
			return true;
		}
	}
	return false;
}

void OP_Manager::setPolicy(PrioritiesList &orderingStrategy) {
	sort(operatingPoints.begin(),
	     operatingPoints.end(),
	     operatingPointsComparator(orderingStrategy));

	vectorId = 0;
}