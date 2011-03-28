/**
 *       @file  resource_accounter_status.h
 *      @brief  Resource Accounter "read-only" status interface
 *
 * This defines an interface providing a set of methods for querying the
 * status of system resources. It is reasonably used by whatever Barbeque
 * component interested in knowing resources status (i.e. the Optimizer).
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

#ifndef BBQUE_RESOURCE_ACCOUNT_STATUS_H_
#define BBQUE_RESOURCE_ACCOUNT_STATUS_H_

#include <memory>
#include <string>

namespace bbque { namespace res {

struct Resource;

/** Shared pointer to Resource descriptor */
typedef std::shared_ptr<Resource> ResourcePtr_t;

/**
 * @class ResourceAccounterStatusIF
 *
 * This definition provide the read-only status interface for interactions
 * between resource accounter and "periferical" components of the RTRM (i.e.
 * the RecipeLoader) for resource information querying.
 */
class ResourceAccounterStatusIF {

public:

	/**
	 * @brief Amount of resource available given an identifying resource path
	 * @param path Resource path
	 * @return The amount of resource available
	 */
	virtual uint64_t Available(std::string const & path) const = 0;

	/**
	 * @brief Total amount of resource given an identifying resource path
	 * @param path Resource path
	 * @return The total amount of resource available
	 */
	virtual uint64_t Total(std::string const & path) const = 0;

	/**
	 * @brief Amount of resource used given an identifying resource path
	 * @param path Resource path
	 * @return The used amount of resource available
	 */
	virtual uint64_t Used(std::string const & path) const = 0;

	/**
	 * @brief Get a resource descriptor
	 * @param path Resource path
	 * @return A shared pointer to the resource descriptor
	 */
	virtual ResourcePtr_t GetResource(std::string const & path) = 0;

	/**
	 * @brief Check the existence of a resource
	 * @param path Resource path
	 * @return True if the resource exists, false otherwise.
	 */
	virtual bool ExistResource(std::string const & path) = 0;

};

}   // namespace res

}   // namespace bbque

#endif  // BBQUE_RESOURCE_ACCOUNT_STATUS_H_

