/**
 *       @file  constraints.h
 *      @brief  Resource constraints asserted by applications
 *
 * This defines the "constraint" entity as a structure through which assert a
 * lower bound, an upper bound, or both, upon a resource usage. A constraint
 * assertion could disable some working mode.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  04/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_CONSTRAINTS_H_
#define BBQUE_CONSTRAINTS_H_

#include <limits>
#include <memory>


namespace bbque { namespace res {
	// Forward declaration
	class Resource;
	// Shared pointer to Resource descriptor
	typedef std::shared_ptr<res::Resource> ResourcePtr_t;
}}

using bbque::res::ResourcePtr_t;


namespace bbque { namespace app {

/**
 * @struct ResourceConstraint
 *
 * This defines resource constraints for the application execution.
 * A constraint assertion could disable some working modes due to an "out of
 * bounds" resource usage. From the scheduer view, only enabled application
 * working modes are taken into account.
 */
struct ResourceConstraint {

	/**
	 * @enum Type of constraint bounds
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


} // namespace app

} // namespace bbque

#endif // BBQUE_CONSTRAINTS_H_

