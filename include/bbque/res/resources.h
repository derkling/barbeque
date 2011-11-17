/**
 *       @file  resources.h
 *      @brief  Classes for managing resource information
 *
 * This defines the classes for instancing objects which manage the resource
 * information: total amount, availability, how many resource are use, which
 * applications are using a specific resource.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  01/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_RESOURCES_H_
#define BBQUE_RESOURCES_H_

#include <cstdint>
#include <cstring>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "bbque/app/application.h"

/** @see WorkingMode BindResource */
#define RSRC_ID_ANY 	-1
#define RSRC_ID_NONE 	-2

using bbque::app::AppPtr_t;

namespace bbque { namespace res {

// Forward declarations
struct Resource;
struct ResourceUsage;
struct ResourceState;

class ResourceAccounter;


/** Shared pointer to Resource descriptor */
typedef std::shared_ptr<Resource> ResourcePtr_t;
/** List of shared pointers to Resource descriptors */
typedef std::list<ResourcePtr_t> ResourcePtrList_t;
/** Shared pointer to ResourceState object */
typedef std::shared_ptr<ResourceState> ResourceStatePtr_t;
/** Map of amounts of resource used by applications. Key: Application UID */
typedef std::map<AppUid_t, uint64_t> AppUseQtyMap_t;
/** Numeric value used as token for the resource views */
typedef size_t RViewToken_t;
/** Hash map collecting the state views of a resource */
typedef std::unordered_map<RViewToken_t, ResourceStatePtr_t> RSHashMap_t;


/**
 * @struct ResourceState
 *
 * It syntetizes the concept of resource state:
 * How many resources are used/available? Which application is using the
 * resource? How much is using it ?
 * This are the basic information we need to track upon resources.
 *
 * @note: The total amount of resource is not an information of "state".
 * Indeed a state is a dynamic concept, while the total is a static
 * information.
 */
struct ResourceState {

	/**
	 * @brief Constructor
	 * @param tot The total amount to set
	 */
	ResourceState():
		used(0) {
	}

	/**
	 * @brief Destructor
	 */
	~ResourceState() {
		apps.clear();
	}

	/** The amount of resource used in the system   */
	uint64_t used;

	/**
	 * Amounts of resource used by each of the applications holding the
	 * resource
	 */
	AppUseQtyMap_t apps;

};


/**
 * @class Resource
 *
 * Of course the "Resource" is the fundamental entity for Barbeque RTRM.
 * To access a resource is a matter of using a "path". A resource path is
 * built recursively, as a sequence of resources name, in a hierarchical
 * form (@see ResourceAccounter, @see ResourceTree).
 *
 * Basically, a resource has a identifying name, a total amount value,  and a
 * state. In our design, MORE than one state.
 * The idea is to have a default state, the "real" one, and a possibile set of
 * temporary states to use as "buffers". Thus each state is a different VIEW
 * of resource. This feature is particularly useful for components like the
 * Scheduler/Optimizer (see below.)
 */
class Resource {

public:

	enum ExitCode_t {
		/** Generic success code */
		RS_SUCCESS = 0,
		/** The resource is not used by any application */
		RS_NO_APPS
	};

	/**
	 * @brief Constructor
	 * @param nm Resource name
	 */
	Resource(std::string const & nm);

	/**
	 * @brief Constructor
	 * @param res_path Resource path
	 * @param tot The total amount of resource
	 */
	Resource(std::string const & res_path, uint64_t tot);

	/**
	 * Destructor
	 */
	~Resource() {
		state_views.clear();
	}

	/**
	 * @brief Resource name
	 * @return The resource name string
	 */
	inline std::string const & Name() {
		return name;
	}

	/**
	 * @brief Set the total amount of resource
	 *
	 * @note This method acts only upon the default state view only
	 * @param tot The amount to set
	 */
	inline void SetTotal(uint64_t tot) {
		total = tot;
	}

	/**
	 * @brief Set the amount of resource used.
	 *
	 * The value must be lesser than total availability.
	 *
	 * @note This method acts only upon the default state view only
	 * @return Return the value of "used". If success, the value will be the
	 * new just set, otherwise the old one.
	 */
	uint64_t SetUsed(uint64_t use);

	/**
	 * @brief Resource total
	 * @return The total amount of resource
	 */
	inline uint64_t Total() {
		return total;
	}

	/**
	 * @brief Amount of resource used
	 *
	 * @param vtok The token referencing the resource view
	 *
	 * @return How much resource has been allocated
	 */
	uint64_t Used(RViewToken_t vtok = 0);

	/**
	 * @brief Resource availablility
	 *
	 * @param vtok The token referencing the resource view
	 *
	 * @return How much resource is still available
	 */
	uint64_t Available(RViewToken_t vtok = 0);

	/**
	 * @brief Resource availability
	 *
	 * @param papp Application interested in the query. We want to include
	 * in the count the amount of resource used by this application.
	 * This could be useful when we want to check the availability with the
	 * aim of allocate the resource to the given application. If the
	 * application is using yet a certain amount of resource this quantity
	 * should be considered as "available" for this application.
	 *
	 * @param vtok The token referencing the resource view
	 *
	 * @return How much resource is still available including the amount of
	 * resource used by the given application
	 */
	uint64_t Available(AppPtr_t & papp, RViewToken_t vtok = 0);

	/**
	 * @brief Count of applications using the resource
	 *
	 * @param vtok The token referencing the resource view
	 * @return Number of applications
	 */
	uint16_t ApplicationsCount(RViewToken_t vtok = 0) {
		AppUseQtyMap_t apps_map;
		return ApplicationsCount(apps_map, vtok);
	}

	/**
	 * @brief Amount of resource used by the application
	 *
	 * @param papp Application (shared pointer) using the resource
	 * @param vtok The token referencing the resource view
	 *
	 * @return The 'quota' of resource used by the application
	 */
	uint64_t ApplicationUsage(AppPtr_t & papp, RViewToken_t vtok = 0);

	/**
	 * @brief Get the Uid of the idx-th App/EXC using the resource
	 *
	 * @param app_uid The Uid of the n-th App/EXC using the resource
	 * @param amount This is set to the amount of resource used by the App/EXC
	 * @param idx The n-th App/EXC to find
	 * @param vtok The token referencing the resource view
	 * @return RS_SUCCESS if the App/EXC has been found, RS_NO_APPS otherwise
	 */
	ExitCode_t UsedBy(AppUid_t & app_uid, uint64_t & amount, uint8_t idx = 0,
			RViewToken_t vtok = 0); 

	/**
	 * @brief Acquire a given amount of resource
	 * @param amount How much resource is required
	 * @param app_ptr The application requiring the resource
	 * @param vtok The token referencing the resource view
	 * @return The amount of resource acquired if success, 0 otherwise.
	 */
	 uint64_t Acquire(uint64_t amount, AppPtr_t const & app_ptr,
			 RViewToken_t vtok = 0);

	/**
	 * @brief Release the resource
	 *
	 * Release the specific amount of resource used by an application
	 *
	 * @param app_ptr The application releasing the resource
	 * @param vtok The token referencing the resource view
	 * @return The amount of resource released
	 */
	uint64_t Release(AppPtr_t const & app_ptr, RViewToken_t vtok = 0);

	/**
	 * @brief Delete a state view
	 *
	 * If the token refers to the default view, the method returns doing
	 * nothing. This control is ahead of safety and consistency purposes.
	 * Indeed if the default view was removed, what state should be picked as
	 * the new default one?
	 * Thus, this constraint force the caller to set a new default view, before
	 * delete the current one.
	 *
	 * @param vtok The token of the view to delete
	 */
	void DeleteView(RViewToken_t vtok);

	/**
	 * @brief The number of state views of the resource
	 * @return The size of the map
	 */
	inline size_t ViewCount() {
		return state_views.size();
	}

private:

	/** Resource name (i.e. "mem0", "pe1", "dma1", ...)        */
	std::string name;

	/** The total amount of resource  */
	uint64_t total;

	/**
	 * Hash map with all the views of the resource.
	 * A "view" is a resource state. We can think at the hash map as a map
	 * containing the "real" state of resource, plus other "temporary" states.
	 * Such temporary states allows the Scheduler/Optimizer, i.e., to make
	 * intermediate evaluations, before commit the ultimate scheduling.
	 *
	 * Each view is identified by a "token" which is hashed in order to
	 * retrieve the ResourceState descriptor.
	 *
	 * It's up to the Resource Accounter to maintain a consistent view of the
	 * system state. Thus ResourceAccounter will manage tokens and the state
	 * views lifecycle.
	 */
	RSHashMap_t state_views;

	/**
	 * @brief Apps/EXCs using the resource
	 *
	 * @param apps_map Reference to the map of App/EXC to get
	 * @param vtok The resource state view token
	 *
	 * @return The number of Apps/EXCs using the resource, and a
	 * reference to the map
	 */
	uint16_t ApplicationsCount(AppUseQtyMap_t & apps_map,
			RViewToken_t vtok = 0);

	/**
	 * @brief Amount of resource used by the application
	 *
	 * @param papp Application (shared pointer) using the resource
	 * @param apps_map Reference to the map of App/EXC to get
	 *
	 * @return The 'quota' of resource used by the application
	 */
	uint64_t ApplicationUsage(AppPtr_t & papp, AppUseQtyMap_t & apps_map);

	/**
	 * @brief Get the view referenced by the token
	 *
	 * @param vtok The resource state view token
	 * @return The ResourceState fo the referenced view
	 */
	ResourceStatePtr_t GetStateView(RViewToken_t vtok);
};


/**
 * @struct ResourceUsage
 *
 * An application working modes defines a set of this resource requests
 * (usages).
 *
 * A resource usage descriptor embeds a couple of information:
 * The first is obviously the value of the usage, the amount of resource
 * requested.
 * The second is a list containing all the descriptors (shared pointer) of the
 * resources which to this usage refers.
 *
 * We expect that such list is filled by a method of ResourceAccounter, after
 * that the Scheduler/Optimizer has solved the resource binding.
 *
 * In other words, if a working mode specifies a request as
 * "arch.tile.cluster2.pe = 4 ", the scheduler/optimizer must define a
 * bind between that "cluster2" and the "real" cluster where to allocate the
 * request of 4 processing elements.
 * The list "binds", after such binding, will contain the descriptors of the
 * resources "pe" in the cluster assigned by the scheduler/optimizer module.
 */
struct ResourceUsage {

	/**
	 * @brief Constructor
	 * @param The amount of resource usage
	 */
	ResourceUsage(uint64_t usage_value):
		value(usage_value) {
	}

	/**
	 * @brief Destructor
	 */
	~ResourceUsage() {
		binds.clear();
	}

	/** Usage value request */
	uint64_t value;

	/** List of resource descriptors which to the resource usage is bind*/
	ResourcePtrList_t binds;

};

}   // namespace res

}   // namespace bbque

#endif // BBQUE_RESOURCES_H_

