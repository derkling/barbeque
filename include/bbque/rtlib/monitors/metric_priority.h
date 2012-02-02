/**
 *      @file   metric_priority.h
 *      @class  MetricPriority
 *      @brief  Filter class definition
 *
 * This class provides a definition of an object used to define a sorting order
 * for metrics. Using a list of these elements, it is possible to define a full
 * ordering of operating points, useful for runtime management purposes.
 *
 *     @author  Vincenzo Consales , vincenzo.consales@gmail.com
 *     @author  Andrea Di Gesare , andrea.digesare@gmail.com
 *
 *   @internal
 *     Created
 *    Revision
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2012, Consales Vincenzo, Di Gesare Andrea
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#ifndef BBQUE_METRIC_PRIORITY_H_
#define BBQUE_METRIC_PRIORITY_H_


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

};

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

#endif /* BBQUE_METRIC_PRIORITY_H_ */
