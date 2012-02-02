/**
 *      @file   op_manager.h
 *      @class  OP_Manager
 *      @brief  Manager of operating points class
 *
 * This class provides a definition of a manager of operating points.
 * It is used to create a structure with all the operating points contained in
 * an xml file and order it according to different metrics. When such a
 * structure is created, it is possible to move between different operating
 * points according to priorities and filters given by the user.
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

#ifndef BBQUE_OP_MANAGER_H_
#define BBQUE_OP_MANAGER_H_

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "operating_point.h"
#include "metric_priority.h"
#include "op_filter.h"

class OP_Manager {

public:

	/**
	 * @brief Constructor of the class
	 *
	 * @param docName Name of the xml file to parse
	 */
	OP_Manager(std::string docName)
			:documentName(docName) {
				parseDoc(documentName);
				vectorId = 0;
	}

	/**
	 * @brief Destructor of the class
	 */
	~OP_Manager();

	/**
	 * @brief Returns the current operating point of the list
	 *
	 * @param op Reference to an OperatingPoint where to save the result
	 * @return True if a point has been found, otherwise False
	 */
	bool getCurrentOP(OperatingPoint &op);

	/**
	 * @brief Returns an higher operating point from the list.
	 *
	 * An higher point is a point that has an higher priority, according to
	 * the ones given by the user.
	 *
	 * @param op Reference to an OperatingPoint where to save the result
	 * @return True if a point has been found, otherwise False
	 */
	bool getHigherOP(OperatingPoint &op);

	/**
	 * @brief Returns a lower operating point from the list.
	 *
	 * An lower point is a point that has a lower priority, according to
	 * the ones given by the user.
	 *
	 * @param op Reference to an OperatingPoint where to save the result
	 * @return True if a point has been found, otherwise False
	 */
	bool getLowerOP(OperatingPoint &op);

	/**
	 * @brief Returns the current operating point from the list that respects
	 * the filtering's criteria
	 *
	 * @param op Reference to an OperatingPoint where to save the result
	 * @param opFilters Reference to a filter list
	 * @return True if a point has been found, otherwise False
	 */
	bool getCurrentOP(OperatingPoint &op, const OP_FilterList &opFilters);

	/**
	 * @brief Returns an higher operating point from the list that respects
	 * the filtering's criteria
	 *
	 * An higher point is a point that has an higher priority, according to
	 * the ones given by the user.
	 *
	 * @param op Reference to an OperatingPoint where to save the result
	 * @param opFilters Reference to a filter list
	 * @return True if a point has been found, otherwise False
	 */
	bool getHigherOP(OperatingPoint &op, const OP_FilterList &opFilters);

	/**
	 * @brief Returns a lower operating point from the list that respects
	 * the filtering's criteria
	 *
	 * An lower point is a point that has a lower priority, according to
	 * the ones given by the user.
	 *
	 * @param op Reference to an OperatingPoint where to save the result
	 * @param opFilters Reference to a filter list
	 * @return True if a point has been found, otherwise False
	 */
	bool getLowerOP(OperatingPoint &op, const OP_FilterList &opFilters);

	/**
	 * @brief Set an ordering policy for operating points
	 *
	 * @param orderingStrategy A list of priorities that will influence the
	 * sorting process. The priorities have to be given in descending order,
	 * with the first element of the list corrisponding the metric with the
	 * highest priority,
	 */
	void setPolicy(PrioritiesList &orderingStrategy);

	/**
	 * @brief Getter for the list of operating points
	 */
	OperatingPointsList getOperatingPoints() const {
		return operatingPoints;
	}

private:
	/**
	 * @brief Name of the xml file to parse
	 */
	std::string documentName;

	/**
	 * @brief Current index of the operating points list
	 */
	uint16_t vectorId;

	/**
	 * @brief List of references to operating points
	 */
	OperatingPointsList operatingPoints;

	/**
	 * @brief Parses the xml document to get all the operating points
	 * The list of operating points is enclosed in the <points></points>
	 * tags
	 *
	 * @param documentName Name of the xml file to parse
	 */
	void parseDoc(std::string documentName);

	/**
	 * @brief Parses a <parameters> node
	 *
	 * @param doc Pointer to the xml document
	 * @param cur Current node
	 * @param operatingPoint Pointer to an OperatingPoint where to save the
	 * result
	 */
	void parseParameters (xmlDocPtr doc,
			      xmlNodePtr cur,
			      OperatingPoint *operatingPoint);

	/**
	 * @brief Parses a <system_metrics> node
	 *
	 * @param doc Pointer to the xml document
	 * @param cur Current node
	 * @param operatingPoint Pointer to an OperatingPoint where to save the
	 * result
	 */
	void parseMetrics (xmlDocPtr doc,
			   xmlNodePtr cur,
			   OperatingPoint *operatingPoint);

	/**
	 * @brief Parses a <point> node
	 *
	 * @param doc Pointer to the xml document
	 * @param cur Current node
	 * @param operatingPoint Pointer to an OperatingPoint where to save the
	 * result
	 */
	void parsePoint (xmlDocPtr doc,
			 xmlNodePtr cur,
			 OperatingPoint *operatingPoint);

	/*
	 * REMEMBER: if a name in the filter is not in the map, it won't be
	 * considered
	 */
	/**
	 * @brief Checks whether an operating point respects some constraints
	 *
	 * @param op Pointer to the OperatingPoint to check
	 * @param opFilters List of constraints to that OperatingPoint
	 * @return True if the point is valid, False otherwise
	 */
	bool isValidOP(OperatingPoint *op, const OP_FilterList &opFilters) const;

	/**
	 * @class operatingPointsComparator
	 * @brief Defines a functor used to sort operating points
	 */
	class operatingPointsComparator{

	public:
		/**
		 * @brief Constructor of the class
		 *
		 * @param orderingStrategy PrioritiesList giving the sorting
		 * order of the operating points
		 */
		operatingPointsComparator(PrioritiesList &orderingStrategy)
				:metricsPriorities(orderingStrategy) {
		}

		/**
		 * @brief Overload of the function call operator, used to
		 * influence the sorting process
		 *
		 * @param op1 Pointer to the first OperatingPoint to compare
		 * @param op2 Pointer to the second OperatingPoint to compare
		 *
		 * @return  True if op1 goes before op2 in the specific strict
		 * weak ordering defined, False otherwise.
		 */
		bool operator()(OperatingPoint *op1, OperatingPoint *op2);

	private:
		/**
		 * @brief PrioritiesList giving the sorting order of the
		 * operating points
		 */
		PrioritiesList metricsPriorities;
	};
};

#endif /* BBQUE_OP_MANAGER_H_ */
