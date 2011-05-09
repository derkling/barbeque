/**
 *       @file  resource_accounter.h
 *      @brief  Resource accounter component for Barbeque RTRM
 *
 * This defines the component for making resource accounting.
 *
 * Each resource of system/platform should be properly registered in the
 * Resource accounter. It keeps track of the information upon availability,
 * total amount and used resources.
 * The information above are updated through proper methods which must be
 * called when an application working mode has triggered.
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

#ifndef BBQUE_RESOURCE_ACCOUNTER_H_
#define BBQUE_RESOURCE_ACCOUNTER_H_

#include <map>
#include <set>

#include "bbque/object.h"
#include "bbque/res/resources.h"
#include "bbque/res/resource_accounter_conf.h"
#include "bbque/res/resource_tree.h"
#include "bbque/utils/utility.h"

#define RESOURCE_ACCOUNTER_NAMESPACE "bq.res_acc"

// Path template for querying clusters state info
#define CLUSTERS_PATH_TEMP "arch.tile.cluster"

using bbque::app::Application;

namespace bbque { namespace res {

/** Shared pointer to ResourceUsage object */
typedef std::shared_ptr<ResourceUsage> UsagePtr_t;
/** Map of ResourceUsage descriptors. Key: resource path */
typedef std::map<std::string, UsagePtr_t> UsagesMap_t;
/** Constant pointer to the map of ResourceUsage descriptors */
typedef UsagesMap_t const * UsagesMapPtr_t;
/** Map of map of ResourceUsage descriptors. Key: application */
typedef std::map<uint32_t, UsagesMapPtr_t> AppUsagesMap_t;
/** Shared pointer to a map of pair Application/ResourceUsages */
typedef std::shared_ptr<AppUsagesMap_t> AppUsagesMapPtr_t;
/** Map of AppUsagesMap_t having the resource state view token as key */
typedef std::map<RViewToken_t, AppUsagesMapPtr_t> AppUsagesViewsMap_t;
/** Set of pointers to the resources allocated under a given state view*/
typedef std::set<ResourcePtr_t> ResourceSet_t;
/** Shared pointer to ResourceSet_t */
typedef std::shared_ptr<ResourceSet_t> ResourceSetPtr_t;
/** Map of ResourcesSetPtr_t. The key is the view token */
typedef std::map<RViewToken_t, ResourceSetPtr_t> ResourceViewsMap_t;


/**
 * @class ResourceAccounter
 *
 * This component is used by the RTRM to do accounting of system resources.
 * Thus ResourceAccounter is in charge of estabilish if a resource usages
 * configuration (defined by the Scheduler/Optimizer module) is valid or not.
 *
 * On the front-side of the accounting mechanisms there are methods through
 * which an application can require a set of resources, using the scheduled
 * working mode as reference.
 *
 * ResourceAccounter keeps track of the state of each resource (amount used,
 * availables, total) and exposes methods for making query.
 *
 * Moreover it exploits the multi-view support of the Resource object. Such
 * feature allow a Scheduler/Optimizer module to do accounting, basing its
 * procedure on a temporary (initially empty) view of the resource states.
 * Once defined a valid configuration, a "commit" can be done. Setting the
 * view defined by the configuration found as the new resources system state.
 */
class ResourceAccounter: public ResourceAccounterConfIF, public Object {

public:

	/**
	 * @brief Return the instance of the ResourceAccounter
	 */
	static ResourceAccounter * GetInstance();

	/**
	 * @brief Destructor
	 */
	~ResourceAccounter();

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Available(std::string const & path, RViewToken_t vtok = 0)
		const {
			ResourcePtrList_t matches = GetResources(path);
			return QueryStatus(matches, RA_AVAIL, vtok);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Total(std::string const & path, RViewToken_t vtok = 0)
		const {
			ResourcePtrList_t matches = GetResources(path);
			return QueryStatus(matches, RA_TOTAL, vtok);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Used(std::string const & path, RViewToken_t vtok = 0)
		const {
			ResourcePtrList_t matches = GetResources(path);
			return QueryStatus(matches, RA_USED, vtok);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline ResourcePtr_t GetResource(std::string const & path) const {
		return resources.find(path);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline ResourcePtrList_t GetResources(std::string const & path) const {
		// If the path is a template find all the resources matching the
		// template. Otherwise do an "hybrid" path based search.
		if (IsPathTemplate(path))
			return resources.findAll(path);
		return resources.findSet(path);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline bool ExistResource(std::string const & path) const {
		std::string _temp_path = PathTemplate(path);
		return resources.existPath(_temp_path);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint16_t ClusteringFactor(std::string const & path) {
		// Check if the resource exists
		if (GetResource(path).get() == 0)
			return 0;
		// Check if the resource is clustered
		int16_t clust_patt_pos = path.find(CLUSTERS_PATH_TEMP);
		if (clust_patt_pos < 0)
			return 1;
		// Compute the factor only if not done yet
		if (clustering_factor == 0) {
			clustering_factor = Total(CLUSTERS_PATH_TEMP);
			// If the query returns 0 the plaform is not cluster-based.
			// Thus we must set clustering factor to 1.
			if (clustering_factor == 0)
				clustering_factor = 1;
		}
		return clustering_factor;
	}

	/**
	 * @see ResourceAccounterConfIF
	 */
	ExitCode_t RegisterResource(std::string const & path,
			std::string const & units, uint64_t amount);

	/**
	 * @see ResourceAccounterConfIF
	 */
	ExitCode_t AcquireUsageSet(Application const * app, RViewToken_t vtok = 0);

	/**
	 * @see ResourceAccounterConfIF
	 */
	void ReleaseUsageSet(Application const * app, RViewToken_t vtok = 0);

	/**
	 * @see ResourceAccounterConfIF
	 */
	RViewToken_t GetNewView(const char * who_req);

	/**
	 * @see ResourceAccounterConfIF
	 */
	void PutView(RViewToken_t tok);

	/**
	 * @see ResourceAccounterConfIF
	 */
	RViewToken_t SetAsSystemState(RViewToken_t vtok);

	/**
	 * @brief Print the resource hierarchy in a tree-like form
	 */
	inline void TreeView() {
		resources.printTree();
	}

private:

	/**
	 * @enum This is used for selecting the state attribute to return from
	 * <tt>queryState()</tt>
	 */
	enum QueryOption_t {
		/** Amount of resource available */
		RA_AVAIL = 0,
		/** Amount of resource used */
		RA_USED,
		/** Total amount of resource */
		RA_TOTAL
	};

	/**
	 * Default constructor
	 */
	ResourceAccounter();

	/**
	 * @brief Return a state parameter (availability, resources used, total
	 * amount) for the resource.
	 *
	 * @param rsrc_set A list of resource descriptors
	 * @param q_opt Resource state attribute requested (@see QueryOption_t)
	 * @param vtok The token referencing the resource state view
	 * @return The value of the attribute request
	 */
	uint64_t QueryStatus(ResourcePtrList_t const & rsrc_set,
				QueryOption_t q_opt, RViewToken_t vtok = 0) const;

	/**
	 * @brief Get a pointer to the map of applications resource usages
	 *
	 * Each application (or better, "execution context") can hold just one set
	 * of resource usages. It's the one defined through the working mode
	 * scheduled. Such assertion is valid inside the scope of the resources
	 * state view referenced by the token.
	 *
	 * @param vtok The token referencing the resource state view
	 * @param apps_usages The map of applications resource usages to get
	 * @return RA_SUCCESS if the map is found. RA_ERR_MISS_VIEW if the token
	 * doesn't match any state view.
	 */
	ExitCode_t GetAppUsagesByView(RViewToken_t vtok,
			AppUsagesMapPtr_t &	apps_usages);

	/**
	 * @brief Increment the resource usages counts
	 *
	 * Each time an application acquires a set of resources (specified in the
	 * working mode scheduled), the counts of resources used must be increased
	 *
	 * @param app_usages Map of next resource usages
	 * @param app The application acquiring the resources
	 * @param vtok The token referencing the resource state view
	 * @return An exit code (@see ExitCode_t)
	 */
	ExitCode_t IncUsageCounts(UsagesMapPtr_t app_usages,
			Application const * app, RViewToken_t vtok = 0);

	/**
	 * @brief Decrement the resource usages counts
	 *
	 * Each time an application releases a set of resources the counts of
	 * resources used must be decreased.
	 *
	 * @param app_usages Map of current resource usages
	 * @param app The application releasing the resources
	 * @param vtok The token referencing the resource state view
	 */
	void DecUsageCounts(UsagesMapPtr_t  app_usages,
			Application const * app, RViewToken_t vtok = 0);

	/** The tree of all the resources in the system.*/
	ResourceTree resources;

	/**
	 * Map containing the pointers to the map of resource usages specified in
	 * the current working modes of each application. The key is the view
	 * token. For each view an application can hold just one set of resource
	 * usages.
	 */
	AppUsagesViewsMap_t usages_per_views;

	/**
	 * Pointer (shared) to the map of applications resource usages, currently
	 * describing the resources system state (default view).
	 */
	AppUsagesMapPtr_t sys_usages_view;

	/**
	 * The token referencing the system resources state (default view).
	 */
	RViewToken_t sys_view_token;

	/**
	 * Keep track of the resources allocated for each view. This data
	 * structure is needed to supports easily a view deletion or to set a view
	 * as the new system state.
	 */
	ResourceViewsMap_t rsrc_per_views;

	/**
	 * Clustering factor. This is equal to the number of clusters in the
	 * platform.
	 */
	uint16_t clustering_factor;

};

}   // namespace res

}   // namespace bbque

#endif  // BBQUE_RESOURCE_ACCOUNTER_H_

