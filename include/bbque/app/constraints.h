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

namespace bbque {

namespace res {
	class Resource;
}

namespace app {

typedef std::shared_ptr<res::Resource> ResourcePtr_t;

/**
 * @struct Constraint
 *
 * It defines constraints for the application execution. When an application
 * asserts a constraint some working modes could became unactive. This means
 * that they cannot be considered by the optimizer in a new scheduling phase.
 */
struct Constraint {

	/**
	 * @enum Type of constraint bounds:0
	 */
	enum BoundType {
		/** Lower bound constraint */
		LOWER_BOUND = 0,
		/** Upper bound constraint */
		UPPER_BOUND
	};

	/** Resource to constraint (shared pointer) */
	ResourcePtr_t resource;

	/** Resource usage upper bound   */
	uint64_t upper;

	/** Resource usage lower bound   */
	uint64_t lower;

};


} // namespace app

} // namespace bbque

#endif // BBQUE_CONSTRAINTS_H_

