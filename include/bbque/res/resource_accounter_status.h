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

#include "bbque/res/resources.h"

// Following macros are defined in order to give a lightweight abstraction
// upon the path template details of some typical resources. The purpose is
// simply to give a more clean way in writing code for Barbeque modules using
// ResourceAccounter calls.

/** System memory */
#define RSRC_SYS_MEM 	"mem"

/** Platform internal memory */
#define RSRC_PLAT_MEM 	"arch.mem"

/** Set of clusters */
#define RSRC_TILE 	"arch.tile"

/** Memory at Tile scope */
#define RSRC_TILE_MEM 	"arch.tile.mem"

/** Cluster of processing element */
#define RSRC_CLUSTER 	"arch.tile.cluster"

/** Memory at Cluster level */
#define RSRC_CLUST_MEM 	"arch.tile.cluster.mem"

/** Processing element of the Cluster */
#define RSRC_CLUST_PE 	"arch.tile.cluster.pe"


using bbque::app::Application;

namespace bbque { namespace res {

/** Type for ID used in resource path */
typedef int16_t ResID_t;
/** Shared pointer to Resource descriptor */
typedef std::shared_ptr<Resource> ResourcePtr_t;
/** List of shared pointer to Resource*/
typedef std::list<ResourcePtr_t> ResourcePtrList_t;
/** Shared pointer to ResourceUsage object */
typedef std::shared_ptr<ResourceUsage> UsagePtr_t;
/** Map of ResourceUsage descriptors. Key: resource path */
typedef std::map<std::string, UsagePtr_t> UsagesMap_t;
/** Constant pointer to the map of ResourceUsage descriptors */
typedef std::shared_ptr<UsagesMap_t> UsagesMapPtr_t;


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
	 * @brief Total amount of resources
	 * @param path Resource path
	 * @return The total amount of resource
	 */
	virtual uint64_t Total(std::string const & path) const = 0;

	/**
	 * @brief Total amount of resources
	 * @param usage A pointer to ResourceUsage
	 * @return The total amount of resource
	 */
	virtual uint64_t Total(UsagePtr_t const & usage) const = 0;

	/**
	 * @brief Amount of resource available
	 * @param path Resource path
	 * @param vtok The token referencing the resource state view
	 * @return The amount of resource available
	 */
	virtual uint64_t Available(std::string const & path, RViewToken_t vtok = 0)
		const = 0;

	/**
	 * @brief Amount of resources available
	 * @param usage A pointer to ResourceUsage
	 * @param vtok The token referencing the resource state view
	 * @return The amount of resource available
	 */
	virtual uint64_t Available(UsagePtr_t const & usage,
			RViewToken_t vtok = 0) const = 0;

	/**
	 * @brief Amount of resources used
	 * @param path Resource path
	 * @param vtok The token referencing the resource state view
	 * @return The used amount of resource
	 */
	virtual uint64_t Used(std::string const & path, RViewToken_t vtok = 0)
		const = 0;

	/**
	 * @brief Amount of resources used
	 * @param usage A pointer to ResourceUsage
	 * @param vtok The token referencing the resource state view
	 * @return The used amount of resource
	 */
	virtual uint64_t Used(UsagePtr_t const & usage, RViewToken_t vtok = 0)
		const = 0;

	/**
	 * @brief Get a resource descriptor
	 * @param path Resource path
	 * @return A shared pointer to the resource descriptor
	 */
	virtual ResourcePtr_t GetResource(std::string const & path) const = 0;

	/**
	 * @brief Get a list of resource descriptors
	 *
	 * Given a "template path" the method return all the resource descriptors
	 * matching such template.
	 * For instance "arch.clusters.cluster.mem" will return all the
	 * descriptors having path "arch.clusters.cluster<N>.mem<M>".
	 *
	 * @param temp_path Template path to match
	 * @return The list of resource descriptors matching the template path
	 */
	virtual ResourcePtrList_t GetResources(std::string const & temp_path)
		const = 0;

	/**
	 * @brief Check the existence of a resource
	 * @param path Resource path
	 * @return True if the resource exists, false otherwise.
	 */
	virtual bool ExistResource(std::string const & path) const = 0;

	/**
	 * @brief App/EXC using a Processing Element resource
	 *
	 * @param path The resource path
	 * @param vtok The token referencing the resource state view
	 * @return A shared pointer to the App/EXC descriptor using the given PE
	 */
	AppPtr_t const AppUsingPE(std::string const & path,
			RViewToken_t vtok = 0) const;

	/**
	 * @brief Clustering factor
	 *
	 * Check if the resource is a clustered one and return the clustering
	 * factor.
	 *
	 * @param path Resource path
	 * @return The number of clusters in the platform if the resource is a
	 * clustered one, 1 if there are no clusters, 0 otherwise.
	 */
	virtual uint16_t ClusteringFactor(std::string const & path) = 0;

	/**
	 * @brief Show the system resources status
	 *
	 * This is an utility function for debug purpose that print out all the
	 * resources path and values about usage and total amount.
	 *
	 * @param vtok Token of the resources state view
	 * @param verbose print in INFO logleve is ture, in DEBUG if false
	 */
	virtual void PrintStatusReport(RViewToken_t vtok = 0,
			bool verbose = false) const = 0;

};

}   // namespace res

}   // namespace bbque

#endif  // BBQUE_RESOURCE_ACCOUNT_STATUS_H_

