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

#include <set>
#include <thread>
#include "bbque/application_manager.h"
#include "bbque/res/resource_accounter_conf.h"
#include "bbque/res/resource_utils.h"
#include "bbque/res/resource_tree.h"
#include "bbque/plugins/logger.h"
#include "bbque/utils/utility.h"

using bbque::ApplicationManager;
using bbque::plugins::LoggerIF;
using bbque::app::AppPtr_t;

#define RESOURCE_ACCOUNTER_NAMESPACE "bq.ra"

// Base path for sync session resource view token
#define SYNC_RVIEW_PATH "ra.sync."

// Max length for the resource view token string
#define TOKEN_PATH_MAX_LEN 30

namespace bbque { namespace res {

/** Map of map of ResourceUsage descriptors. Key: Application UID*/
typedef std::map<AppUid_t, UsagesMapPtr_t> AppUsagesMap_t;
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
class ResourceAccounter: public ResourceAccounterConfIF {

public:

	/**
	 * @brief Return the instance of the ResourceAccounter
	 */
	static ResourceAccounter & GetInstance();

	/**
	 * @brief Destructor
	 */
	~ResourceAccounter();

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Total(std::string const & path) const {
		ResourcePtrList_t matches = GetResources(path);
		return QueryStatus(matches, RA_TOTAL, 0);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Total(UsagePtr_t const & usage_ptr) const {
		if (usage_ptr->binds.empty())
			return 0;
		return QueryStatus(usage_ptr->binds, RA_TOTAL, 0);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Available(std::string const & path,
			RViewToken_t vtok = 0) const {
		ResourcePtrList_t matches = GetResources(path);
		return QueryStatus(matches, RA_AVAIL, vtok);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Available(UsagePtr_t const & usage_ptr,
			RViewToken_t vtok = 0) const {
		if (usage_ptr->binds.empty())
			return 0;
		return QueryStatus(usage_ptr->binds, RA_AVAIL, vtok);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Used(std::string const & path,
			RViewToken_t vtok = 0) const {
		ResourcePtrList_t matches = GetResources(path);
		return QueryStatus(matches, RA_USED, vtok);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Used(UsagePtr_t const & usage_ptr,
			RViewToken_t vtok = 0) const {
		if (usage_ptr->binds.empty())
			return 0;
		return QueryStatus(usage_ptr->binds, RA_USED, vtok);
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
	uint16_t ClusteringFactor(std::string const & path);

	/**
	 * @see ResourceAccounterStatusIF
	 */
	void PrintStatusReport(RViewToken_t vtok = 0, bool verbose = false) const;

	/**
	 * @see ResourceAccounterConfIF
	 */
	ExitCode_t RegisterResource(std::string const & path,
			std::string const & units, uint64_t amount);

	/**
	 * @see ResourceAccounterConfIF
	 */
	ExitCode_t BookResources(AppPtr_t papp, UsagesMapPtr_t const & usages,
			RViewToken_t vtok = 0, bool do_check = true);

	/**
	 * @see ResourceAccounterConfIF
	 */
	void ReleaseResources(AppPtr_t papp, RViewToken_t vtok = 0);

	/**
	 * @see ResourceAccounterConfIF
	 */
	ExitCode_t GetView(std::string who_req, RViewToken_t & tok);

	/**
	 * @see ResourceAccounterConfIF
	 */
	void PutView(RViewToken_t tok);


	/**
	 * @brief Print the resource hierarchy in a tree-like form
	 */
	inline void TreeView() {
		resources.printTree();
	}

	/**
	 * @see ResourceAccounterConfIF
	 */
	ExitCode_t SyncStart();

	/**
	 * @see ResourceAccounterConfIF
	 */
	ExitCode_t SyncAcquireResources(AppPtr_t const & papp,
			UsagesMapPtr_t const & usages);

	/**
	 * @see ResourceAccounterConfIF
	 */
	void SyncAbort();

	/**
	 * @see ResourceAccounterConfIF
	 */
	ExitCode_t SyncCommit();


private:

	/**
	 * @enum This is used for selecting the state attribute to return from
	 * <tt>QueryStatus()</tt>
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
	 * @struct SyncSession_t
	 * @brief Store info about a synchronization session
	 */
	struct SyncSession_t {
		/** Mutex for protecting the session */
		std::mutex mtx;
		/** If true a synchronization session has started */
		bool started;
		/** Token for the temporary resource view */
		RViewToken_t view;
		/** Count the number of session elapsed */
		uint32_t count;

	} sync_ssn;

	/** The logger used by the resource accounter */
	LoggerIF  *logger;

	/** The Application Manager component */
	bbque::ApplicationManager & am;
	
	/** Mutex protecting resource release and acquisition */
	std::recursive_mutex status_mtx;

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
	 * @brief Check the resource availability for a whole set
	 *
	 * @param usages A map of ResourceUsage objects to check
	 * @param vtok The token referencing the resource state view
	 * @return RA_SUCCESS if all the resources are availables,
	 * RA_ERR_USAGE_EXC otherwise.
	 */
	ExitCode_t CheckAvailability(UsagesMapPtr_t const & usages,
			RViewToken_t vtok = 0) const;

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
	 */
	void IncBookingCounts(UsagesMapPtr_t const & app_usages,
			AppPtr_t const & papp, RViewToken_t vtok = 0);

	/**
	 * @brief Book a single resource
	 *
	 * Divide the usage amount between the resources referenced in the "binds"
	 * list.
	 *
	 * @param papp The Application/ExC using the resource
	 * @param rsrc_usage ResourceUsage object
	 * @param vtok The token referencing the resource state view
	 * @param rsrcs_pèr_view Set of resources used in the view specified
	 */
	ExitCode_t DoResourceBooking(AppPtr_t const & papp,
			UsagePtr_t & rsrc_usage, RViewToken_t vtok,
			ResourceSetPtr_t & rsrcs_per_view);

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
	void DecBookingCounts(UsagesMapPtr_t const & app_usages,
			AppPtr_t const & app, RViewToken_t vtok = 0);

	/**
	 * @brief Unbook a single resource
	 *
	 * Remove the amount of usage from the resources referenced in the "binds"
	 * list.
	 *
	 * @param papp The Application/ExC using the resource
	 * @param rsrc_usage ResourceUsage object
	 * @param vtok The token referencing the resource state view
	 * @param rsrcs_pèr_view Set of resources used in the view specified
	 */
	void UndoResourceBooking(AppPtr_t const & papp, UsagePtr_t & rsrc_usage,
			RViewToken_t vtok, ResourceSetPtr_t & rsrcs_per_view);

	/**
	 * @brief Init the synchronized mode session
	 *
	 * This inititalizes the sync session view by adding the resource usages
	 * of the RUNNING Applications/ExC. Thus the ones that will not be
	 * reconfigured or migrated.
	 *
	 * @return @see ExiTCode_t
	 */
	ExitCode_t SyncInit();

	/**
	 * @brief Finalize the synchronized mode
	 *
	 * Safely close the sync session by releasing the mutex and unsetting the
	 * "started" flag.
	 */
	inline void SyncFinalize() {
		sync_ssn.started = false;
		sync_ssn.mtx.unlock();
	}

	/**
	 * @brief Set a view as the new resources state of the system
	 *
	 * Set a new system state view means that for each resource used in that
	 * view, such view becomes the default one.
	 * This is called once a new scheduling has performed, Applications/ExC
	 * must be syncronized, and thus system resources state  must be update
	 *
	 * @param tok The token used as reference to the resources view.
	 * @return The token referencing the system state view.
	 */
	RViewToken_t SetView(RViewToken_t vtok);

	/** The tree of all the resources in the system.*/
	ResourceTree resources;

	/** The set of all the resource paths registered */
	std::set<std::string> paths;

	/** Keep track of the max length between resources path string */
	uint8_t path_max_len;

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

};

}   // namespace res

}   // namespace bbque

#endif  // BBQUE_RESOURCE_ACCOUNTER_H_

