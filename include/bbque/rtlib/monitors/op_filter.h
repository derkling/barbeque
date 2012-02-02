/**
 *      @file   op_filter.h
 *      @class  OP_Filter
 *      @brief  Filter class definition
 *
 * This class provides a definition of a general filter on operating points.
 * Each filter consist of the name of a metric, a comparison function and a
 * value (eg. "Power" LessOrEqual 10). Using a list of filters such as
 * OP_FilterList, it's possible to filter out from a set of operating poins the
 * unwanted ones.
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

};

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
