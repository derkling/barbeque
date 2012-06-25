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

#ifndef BBQUE_METRIC_PRIORITY_H_
#define BBQUE_METRIC_PRIORITY_H_

namespace bbque { namespace rtlib { namespace as {

/**
 * @brief Useful redefinition of a comparison function (via a functor) used by
 * std::sort
 */
typedef std::function<bool(double, double)> SortingFunction;

/**
 * @brief Contains useful functors needed for the sort function
 */
namespace SortingOrder {

const SortingFunction LowestToHighest = std::less<double>();
const SortingFunction HighestToLowest = std::greater<double>();

}

/**
 * @brief Sorting order for metrics
 * @ingroup rtlib_sec04_op
 *
 * @details
 * This class provides a definition of an object used to define a sorting order
 * for metrics. Using a list of these elements, it is possible to define a full
 * ordering of operating points, useful for runtime management purposes.
 */
class MetricPriority{
public:
	/**
	 * @brief Name of the metric
	 */
	std::string metricName;

	/**
	 * @brief Functor representing the comparison function used by sort
	 */
	SortingFunction comparisonFunction;

	/**
	 * @brief Constructor of the class
	 *
	 * @param mName Name of the metric
	 * @param cFun Functor representing the comparison function used by sort
	 */
	MetricPriority(std::string mName, SortingFunction cFun)
			:metricName(mName),comparisonFunction(cFun){
	}
};

/**
 * @brief Defines a type for a vector of MetricPriority
 */
typedef std::vector<MetricPriority> PrioritiesList;

} // namespace as

} // namespace rtlib

} // namespace bbque

#endif /* BBQUE_METRIC_PRIORITY_H_ */
