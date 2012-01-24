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
	inline uint64_t Total(ResourcePtrList_t & rsrc_list) const {
		if (rsrc_list.empty())
			return 0;
		return QueryStatus(rsrc_list, RA_TOTAL);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Available(std::string const & path,
			RViewToken_t vtok = 0, AppPtr_t papp = AppPtr_t()) const {
		ResourcePtrList_t matches = GetResources(path);
		return QueryStatus(matches, RA_AVAIL, vtok, papp);
	}

	/**
	 * @see ResourceAccounterStatusIF
	 */
	inline uint64_t Available(ResourcePtrList_t & rsrc_list,
			RViewToken_t vtok = 0, AppPtr_t papp = AppPtr_t()) const {
		if (rsrc_list.empty())
			return 0;
		return QueryStatus(rsrc_list, RA_AVAIL, vtok, papp);
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
	inline uint64_t Used(ResourcePtrList_t & rsrc_list,
			RViewToken_t vtok = 0) const {
		if (rsrc_list.empty())
			return 0;
		return QueryStatus(rsrc_list, RA_USED, vtok);
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
	AppPtr_t const AppUsingPE(std::string const & path,
			RViewToken_t vtok = 0) const;

	/**
	 * @see ResourceAccounterStatusIF
	 */
	uint16_t ClusteringFactor(std::string const & path);

	/**
	 * @see ResourceAccounterStatusIF
	 */
	void PrintStatusReport(RViewToken_t vtok = 0, bool verbose = false) const;

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
	 * "arch.mem0 	: internal memory (to the platform)
	 * "arch.tile.cluster2 		: cluster 2 of the platform
	 * "arch.tile.cluster0.pe1 	: processing element 1 in cluster 0
	 *
	 * @param path Resource path
	 * @param units Units for the amount value (i.e. "1", "Kbps", "Mb", ...)
	 * @param amount The total amount available
	 *
	 * @return RA_SUCCESS if the resource has been successfully registered.
	 * RA_ERR_MISS_PATH if the path string is empty. RA_ERR_MEM if the
	 * resource descriptor cannot be allocated.
	 */
	ExitCode_t RegisterResource(std::string const & path,
			std::string const & units, uint64_t amount);

	/**
	 * @brief Book e a set of resources
	 *
	 * The method first check that the application doesn't hold another
	 * resource set, then check thier availability, and finally reserves, for
	 * each resource in the usages map specified, the required quantity.
	 *
	 * @param app The application requiring resource usages
	 * @param usages Map of ResourceUsage objects
	 * @param vtok The token referencing the resource state view
	 * @param do_check If true the controls upon set validity and resources
	 * availability are enabled
	 *
	 * @return RA_SUCCESS if the operation has been successfully performed.
	 * RA_ERR_MISS_APP if the application descriptor is null.
	 * RA_ERR_MISS_USAGES if the resource usages map is empty.
	 * RA_ERR_MISS_VIEW if the resource state view referenced by the given
	 * token cannot be retrieved.
	 * RA_ERR_USAGE_EXC if the resource set required is not completely
	 * available.
	 */
	ExitCode_t BookResources(AppPtr_t papp,
			UsagesMapPtr_t const & rsrc_usages, RViewToken_t vtok = 0,
			bool do_check = true);

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
	 * @brief Get the system resource state view
	 *
	 * @return The token of the system view
	 */
	inline RViewToken_t GetSystemView() {
		return sys_view_token;
	}

	/**
	 * @brief Get the scheduled resource state view
	 *
	 * @return The token of the scheduled view
	 */
	inline RViewToken_t GetScheduledView() {
		return sch_view_token;
	}

	/**
	 * @brief Set the scheduled resource state view
	 *
	 * @return The token of the system view
	 */
	inline void SetScheduledView(RViewToken_t svt) {
		RViewToken_t old_svt = sch_view_token;
		// First updated the new scheduled view
		sch_view_token = svt;
		// ... than we can keep time to release the previous one
		if (old_svt != sys_view_token)
			// but this is to be done only if the previous view was not the
			// current system view
			PutView(old_svt);
	}

	/**
	 * @brief Print the resource hierarchy in a tree-like form
	 */
	inline void TreeView() {
		resources.printTree();
	}

	/**
	 * @brief Start a synchronized mode session
	 *
	 * Once a scheduling/resource allocation has been performed we need to
	 * make the changes effective, by updating the system resources state.
	 * For doing that a "synchronized mode session" must be started. This
	 * method open the session and init a new resource view by computing the
	 * resource accounting info of the Applications/ExC having a "RUNNING"
	 * scheduling state (the ones that continue running without
	 * reconfiguration or migrations).
	 *
	 * @return @see ExitCode_t
	 */
	ExitCode_t SyncStart();

	/**
	 * @brief Acquire resources for an Application/ExC
	 *
	 * If the sync session is not open does nothing. Otherwise it does
	 * resource booking using the state view allocated for the session.
	 * The resource set is retrieved from the "next AWM".
	 *
	 * @param papp Application/ExC acquiring the resources
	 *
	 * @return @see ExitCode_t
	 */
	ExitCode_t SyncAcquireResources(AppPtr_t const & papp);

	/**
	 * @brief Abort a synchronized mode session
	 *
	 * Changes are trashed away. The resource bookings performed in the
	 * session are canceled.
	 */
	void SyncAbort();

	/**
	 * @brief Commit a synchronized mode session
	 *
	 * Changes are made effective. Resources must be allocated accordingly to
	 * the state view built during in the session.
	 *
	 * @return @see ExitCode_t
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
	 * Keep track of the resources allocated for each view. This data
	 * structure is needed to supports easily a view deletion or to set a view
	 * as the new system state.
	 */
	ResourceViewsMap_t rsrc_per_views;

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
	 * @brief Token referencing the view on scheduled resources
	 *
	 * When a new scheduling has been selected, this is the token referencing
	 * the corresponding view on resources state. When that schedule has been
	 * committed, i.e. resources usage synchronized, this has the same value
	 * of sys_view_token.
	 * 
	 */
	RViewToken_t sch_view_token;

	/**
	 * Default constructor
	 */
	ResourceAccounter();

	/**
	 * @brief Report string about applications using PEs
	 *
	 * @param path The path of the PE resource in use
	 * @param buff The string buffer to set
	 * @param size The buffer size
	 * @param vtok The token referencing the resource state view
	 * @return A report string with the App/EXC id info, the priority and the
	 * current AWM
	 */
	char const * StrAppUsingPE(std::string const & path, char * buff,
			size_t	size, RViewToken_t vtok = 0) const;

	/**
	 * @brief Return a state parameter (availability, resources used, total
	 * amount) for the resource.
	 *
	 * @param rsrc_list A list of descriptors of resources of the same type
	 * @param q_opt Resource state attribute requested (@see QueryOption_t)
	 * @param vtok The token referencing the resource state view
	 * @param papp The application interested in the query
	 *
	 * @return The value of the attribute request
	 */
	uint64_t QueryStatus(ResourcePtrList_t const & rsrc_list,
				QueryOption_t q_opt, RViewToken_t vtok = 0,
				AppPtr_t papp =	AppPtr_t()) const;

	/**
	 * @brief Check the resource availability for a whole set
	 *
	 * @param usages A map of ResourceUsage objects to check
	 * @param vtok The token referencing the resource state view
	 * @param papp The application interested in the query
	 * @return RA_SUCCESS if all the resources are availables,
	 * RA_ERR_USAGE_EXC otherwise.
	 */
	ExitCode_t CheckAvailability(UsagesMapPtr_t const & usages,
			RViewToken_t vtok = 0, AppPtr_t papp = AppPtr_t()) const;

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
	 * @param pusage ResourceUsage object
	 * @param vtok The token referencing the resource state view
	 *
	 * @return RA_ERR_USAGE_EXC if the usage required overcome the
	 * availability. RA_SUCCESS otherwise.
	 */
	ExitCode_t DoResourceBooking(AppPtr_t const & papp,
			UsagePtr_t & pusage, RViewToken_t vtok);

	/**
	 * @brief Allocate a quota of resource in the scheduling case
	 *
	 * Allocate a quota of the required resource in a single resource binding
	 * taken from the "binds" list of the ResourceUsage associated.
	 *
	 * @param papp The Application/ExC using the resource
	 * @param rsrc The resource descriptor of the resource binding
	 * @param usage_val The amount of resource required
	 * @param vtok The token referencing the resource state view
	 */
	void SchedResourceBooking(AppPtr_t const & papp, ResourcePtr_t & rsrc,
			uint64_t & usage_val, RViewToken_t vtok);

	/**
	 * @brief Allocate a quota of resource in the synchronization case
	 *
	 * Allocate a quota of the required resource in a single resource binding
	 * taken checking the assigments done by the scheduler. To retrieve this
	 * information, the scheduled view is properly queried.
	 *
	 * @param papp The Application/ExC using the resource
	 * @param rsrc The resource descriptor of the resource binding
	 * @param usage_val The amount of resource required
	 */
	void SyncResourceBooking(AppPtr_t const & papp, ResourcePtr_t & rsrc,
			uint64_t & usage_val);

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
	 * @param pusage ResourceUsage object
	 * @param vtok The token referencing the resource state view
	 */
	void UndoResourceBooking(AppPtr_t const & papp, UsagePtr_t & pusage,
			RViewToken_t vtok);

	/**
	 * @brief Init the synchronized mode session
	 *
	 * This inititalizes the sync session view by adding the resource usages
	 * of the RUNNING Applications/ExC. Thus the ones that will not be
	 * reconfigured or migrated.
	 *
	 * @return RA_ERR_SYNC_INIT if the something goes wrong in the assignment
	 * resources to the previuosly running applications/EXC.
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

};

}   // namespace res

}   // namespace bbque

#endif  // BBQUE_RESOURCE_ACCOUNTER_H_

