/**
 *       @file  resource_accounter_conf.h
 *      @brief  "Update" ResourceAccounter status interface
 *
 * This defines the "update" status interface for ResourceAccounter.
 * This interface allows the update of resource states information. For
 * instance, when the Application Manager receive a notify upon an Application
 * working mode switch. In such case an update about the resource usages is
 * needed.
 * Moreover when Barbeque check for the platform resources, we need a method
 * for register the resources, filling a descriptor with the information
 * exposed by the platform.
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

#ifndef BBQUE_RESOURCE_ACCOUNTER_CONF_IF_H_
#define BBQUE_RESOURCE_ACCOUNTER_CONF_IF_H_

#include "bbque/res/resource_accounter_status.h"

using bbque::app::AppPtr_t;
using bbque::app::Application;

namespace bbque { namespace res {

/**
 * @class ResourceAccounterConfIF
 *
 * This interface allow the update of runtime resource information into the
 * Resource Accounter component.
 */
class ResourceAccounterConfIF: public ResourceAccounterStatusIF {

public:

	/**
	 * @enum Exit codes
	 */
	enum ExitCode_t {
		/** Successful return  */
		RA_SUCCESS = 0,
		/** Argument "path" missing */
		RA_ERR_MISS_PATH,
		/** Unable to allocate a new resource descriptor */
		RA_ERR_MEM,
		/** Unable to find the state view specified */
		RA_ERR_MISS_VIEW,
		/** Application reference missing */
		RA_ERR_MISS_APP,
		/** Resource usages map missing	 */
		RA_ERR_MISS_USAGES,
		/** Application uses yet another resource set */
		RA_ERR_APP_USAGES,
		/** Resource usage required exceeds the availabilities */
		RA_ERR_USAGE_EXC
	};

	/**
	 * @brief Register a resource
	 *
	 * Setup informations about a resource installed into the system.
	 * Resources can be system-wide or placed on the platform. A resource is
	 * identified by its pathname. The latters for instance must have the
	 * "arch." suffix in front. While the formers doesn't show any suffix.
	 *
	 * Thus we could have resource paths as below :
	 *
	 * "mem0" 		: system memory
	 * "spi0" 		: SPI system bus
	 * "arch.mem0 	: internal memory (to the platform)
	 * "arch.tile.cluster2 		: cluster 2 of the platform
	 * "arch.tile.cluster0.pe1 	: processing element 1 in cluster 0
	 *
	 * @param path Resource path
	 * @param units Units for the amount value (i.e. "1", "Kbps", "Mb", ...)
	 * @param amount The total amount available
	 * @return An exit code (@see ExitCode_t)
	 */
	virtual ExitCode_t RegisterResource(std::string const & path,
			std::string const & units, uint64_t amount) = 0;

	/**
	 * @brief Acquire a set of resources
	 *
	 * The method first check that the application doesn't hold another
	 * resource set. If so such resources are released. Then it reserves, for
	 * each resource in the usages map specified, the required quantity.
	 *
	 * @param app The application requiring resource usages
	 * @oparam usages Map of ResourceUsage objects
	 * @param vtok The token referencing the resource state view
	 * @return An exit code (@see ExitCode_t)
	 */
	virtual ExitCode_t BookResources(AppPtr_t app,
			UsagesMapPtr_t const & usages, RViewToken_t vtok = 0) = 0;

	/**
	 * @brief Release the resources
	 *
	 * The method is typically called when an application stops running
	 * (exit or is killed) and releases all the resource usages.
	 * It lookups the current set of resource usages of the application and
	 * release it all.
	 *
	 * @param app The application holding the resources
	 * @param vtok The token referencing the resource state view
	 */
	virtual void ReleaseResources(AppPtr_t app, RViewToken_t vtok = 0) = 0;

	/**
	 * @brief Get a new resources view
	 *
	 * A component (core or module) can require a "personal" view of the
	 * resources. This means that ResourceAccounter "virtually clones"
	 * the system resources, blanking their states, allowing the component to
	 * make accounting without modify the real state of the resources.
	 *
	 * The component (i.e. Scheduler/Optimizer) should use the token returned
	 * with all the accounting methods, as a reference to the considered
	 * resources view. Note that a requiring component can manage more than
	 * one view.
	 *
	 * @param who_req A string identifying who requires the resource view
	 * @param tok The token to return for future references to the view
	 * @return @see ExitCode_t
	 */
	virtual ExitCode_t GetView(const char * who_req, RViewToken_t & tok) = 0;

	/**
	 * @brief Release a resources state view
	 *
	 * Remove the resources state view referenced by the token number.
	 *
	 * @param tok The token used as reference to the resources state view.
	 */
	virtual void PutView(RViewToken_t tok) = 0;

	/**
	 * @see Set a view as the new resources state of the system
	 *
	 * Set a new system state view means that for each resource used in that
	 * view, such view becomes the default one.
	 * This is how the Scheduler/Optimizer has defined the resource assignment
	 * to the running applications.
	 *
	 * @param tok The token used as reference to the resources view.
	 * @return The token referencing the system state view.
	 */
	virtual RViewToken_t SetView(RViewToken_t vtok) = 0;

};

} // namespace res

} // namespace bbque

#endif // BBQUE_RESOURCE_ACCOUNTER_CONF_IF_H_

