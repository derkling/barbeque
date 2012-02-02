/**
 *      @file   operating_point.h
 *      @class  OperatingPoint
 *      @brief  Operating point class definition
 *
 * This class provides a definition of a general operating point.
 * Every operating point is defined by its own application parameters and a set
 * of metrics that describe the performance/behavior obtained using that set.
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

#ifndef BBQUE_OPERATING_POINT_H_
#define BBQUE_OPERATING_POINT_H_

#include <map>
#include <string>

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
};

/**
 * @brief Defines a type for a vector of OperatingPoint pointers
 */
typedef std::vector<OperatingPoint*> OperatingPointsList;

#endif /* BBQUE_OPERATING_POINT_H_ */
