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

#ifndef BBQUE_OP_FILTER_H_
#define BBQUE_OP_FILTER_H_

#include <string>
#include <vector>

/**
 * @brief Useful definition for a comparison function (via a functor)
 */
typedef std::function<bool(double, double)> ComparisonFunctor;

/**
 * @brief Contains useful functors needed for comparison purposes with names
 * easy to remember
 */
namespace ComparisonFunctors{

const ComparisonFunctor Less = std::less<double>();
const ComparisonFunctor Greater = std::greater<double>();
const ComparisonFunctor LessOrEqual = std::less_equal<double>();
const ComparisonFunctor GreaterOrEqual = std::greater_equal<double>();

}

class OP_Filter{
public:
	/**
	 * @brief Name of the metric to filter
	 */
	std::string name;

	/**
	 * @brief Functor representing the comparison function used in the
	 * filtering process
	 */
	ComparisonFunctor cFunction;

	/**
	 * @brief Upper or Lower bound for the metric
	 */
	double value;

	/**
	 * @brief Constructor of the class
	 *
	 * @param name Name of the metric to filter
	 * @param cFunction Functor representing the comparison function used in
	 *  the filtering process
	 * @param value Upper or Lower bound for the metric
	 */
	OP_Filter(std::string name, ComparisonFunctor cFunction, double value):
			 name(name), cFunction(cFunction), value(value) {

	}
};

/**
 * @brief Defines a type for a vector of filters
 */
typedef std::vector<OP_Filter> OP_FilterList;

#endif /* BBQUE_OP_FILTER_H_ */
