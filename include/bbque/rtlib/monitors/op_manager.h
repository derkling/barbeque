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

#ifndef BBQUE_OP_MANAGER_H_
#define BBQUE_OP_MANAGER_H_

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "operating_point.h"
#include "metric_priority.h"
#include "op_filter.h"


class OP_Manager {

public:

	/**
	 * @brief Constructor of the class
	 */
	OP_Manager() {
		vectorId = 0;
		isHighestOP = true;
	}

	/**
	 * @brief Constructor of the class
	 *
	 * @param opList List of operating points
	 */
	OP_Manager(OperatingPointsList opList)
			: operatingPoints(opList){
				vectorId = 0;
				isHighestOP = true;
	}

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

	//TODO Add description
	bool highestOP() {
		return isHighestOP;
	}

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
	 * @brief Current index of the operating points list
	 */
	uint16_t vectorId;

	/**
	 * @brief Shows if the manager is pointing to the highest available
	 * op in that moment.
	 */
	bool isHighestOP;

	/**
	 * @brief List of references to operating points
	 */
	OperatingPointsList operatingPoints;

	/**
	 * @brief Checks whether an operating point respects some constraints
	 *
	 * REMEMBER: if a name in the filter is not in the map, it won't be
	 * considered
	 *
	 * @param op Pointer to the OperatingPoint to check
	 * @param opFilters List of constraints to that OperatingPoint
	 * @return True if the point is valid, False otherwise
	 */
	bool isValidOP(const OperatingPoint &op,
		       const OP_FilterList &opFilters) const;

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
		bool operator()(const OperatingPoint &op1,
				const OperatingPoint &op2) const;

	private:
		/**
		 * @brief PrioritiesList giving the sorting order of the
		 * operating points
		 */
		PrioritiesList metricsPriorities;
	};
};

/*******************************************************************************
 *    Doxygen Module Documentation
 ******************************************************************************/

/**
 * @defgroup rtlib_sec04_op Application-Specific Operating Points Support
 * @ingroup rtlib_sec04_rtrm
 *
 * ADD MORE DETAILS HERE (Monitors)
 *
 */

#endif /* BBQUE_OP_MANAGER_H_ */
