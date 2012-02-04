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

#ifndef BBQUE_CONSTRAINTS_H_
#define BBQUE_CONSTRAINTS_H_

#include <limits>
#include <memory>

namespace bbque { namespace res {


// Forward declaration
class Resource;
// Shared pointer to Resource descriptor
typedef std::shared_ptr<Resource> ResourcePtr_t;

/**
 * @brief Constraints asserted on resources
 *
 * This defines resource constraints for the application execution.
 * A constraint assertion could disable some working modes due to an "out of
 * bounds" resource usage. From the scheduer view, only enabled application
 * working modes are taken into account.
 */
struct ResourceConstraint {

	/**
	 * @brief Type of constraint bounds
	 */
	enum BoundType_t {
		/** Lower bound constraint */
		LOWER_BOUND = 0,
		/** Upper bound constraint */
		UPPER_BOUND
	};

	ResourceConstraint():
		lower(0),
		upper(std::numeric_limits<uint64_t>::max()) {
	}

	/**
	 * @brief Constructor with resource descriptor
	 * @param rsrc_ptr Resource descriptor pointer
	 */
	ResourceConstraint(ResourcePtr_t const & rsrc_ptr):
		resource(rsrc_ptr),
		lower(0),
		upper(std::numeric_limits<uint64_t>::max()) {
	}

	/**
	 * @brief Constructor with init values
	 * @param lb Lower bound value
	 * @param ub Upper bound value
	 */
	ResourceConstraint(uint64_t lb, uint64_t ub):
		lower(lb),
		upper(ub) {
	}

	/**
	 * Resource to constraint (shared pointer)
	 * NOTE: This attribute is not used in the current version (0.6)
	 */
	ResourcePtr_t resource;

	/** Resource usage lower bound   */
	uint64_t lower;

	/** Resource usage upper bound   */
	uint64_t upper;

};


} // namespace res

} // namespace bbque

#endif // BBQUE_CONSTRAINTS_H_
