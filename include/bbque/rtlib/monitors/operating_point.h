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

#ifndef BBQUE_OPERATING_POINT_H_
#define BBQUE_OPERATING_POINT_H_

#include <map>
#include <string>
#include <vector>

class OperatingPoint{
public:
	/**
	 * @brief List of parameters identified by their names and values
	 */
	std::map<std::string, double>  parameters;

	/**
	 * @brief List of metrics identified by their names and values
	 */
	std::map<std::string, double>  metrics;


	OperatingPoint(){

	}

	OperatingPoint(std::map<std::string, double> parameters,
		       std::map<std::string, double>  metrics) :
			       parameters(parameters),
			       metrics(metrics) {
	}
};

/**
 * @brief Defines a type for a vector of OperatingPoint pointers
 */
typedef std::vector<OperatingPoint> OperatingPointsList;

#endif /* BBQUE_OPERATING_POINT_H_ */
