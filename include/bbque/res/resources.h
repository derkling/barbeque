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

using bbque::app::Application;

namespace bbque { namespace res {

// Forward declarations
struct Resource;
struct ResourceUsage;
struct ResourceState;


/** Shared pointer to Resource descriptor */
typedef std::shared_ptr<Resource> ResourcePtr_t;
/** List of shared pointers to Resource descriptors */
typedef std::list<ResourcePtr_t> ResourcePtrList_t;
/** Shared pointer to ResourceState object */
typedef std::shared_ptr<ResourceState> ResourceStatePtr_t;
/** Map of amounts of resource used by applications. Key: Application PID */
typedef std::map<uint32_t, uint64_t> AppUseQtyMap_t;
/** Numeric value used as token for the resource views */
typedef size_t RViewToken_t;
/** Hash map collecting the state views of a resource */
typedef std::unordered_map<RViewToken_t, ResourceStatePtr_t> RSHashMap_t;


/** Map of Application descriptor pointers. Key: application name */
typedef std::map<std::string, Application const *> AppMap_t;

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

	/**
	 * @brief Constructor
	 * @param nm Resource name
	 */
	Resource(std::string const & nm):
		name(nm),
		default_key(0) {
		// Init the default view
		default_view = ResourceStatePtr_t(new ResourceState());
		state_views[default_key] = default_view;
	}

	/**
	 * @brief Constructor
	 * @param res_path Resource path
	 * @param tot The total amount of resource
	 */
	Resource(std::string const & res_path, uint64_t tot):
		total(tot),
		default_key(0) {

		// Init the default view
		default_view = ResourceStatePtr_t(new ResourceState());
		state_views[0] = default_view;

		// Extract the name from the path
		size_t pos = res_path.find_last_of(".");
		if (pos != std::string::npos)
			name = res_path.substr(pos + 1);
		else
			name = res_path;
	}

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
	inline uint64_t SetUsed(uint64_t use) {
		if (use <= total)
			default_view->used = use;
		return default_view->used;
	}

	/**
	 * @brief Put the amount of resource used equal to 0.
	 * @note This method acts only upon the default state view only
	 */
	inline void ResetUsed() {
		default_view->used = 0;
	}

	/**
	 * @brief Resource total
	 * @return The total amount of resource
	 */
	inline uint64_t Total() {
		return total;
	}

	/**
	 * @brief Resource used
	 * @param vtok The token referencing the resource view
	 * @return Amount of resource used
	 */
	inline uint64_t Used(RViewToken_t vtok = 0) {
		// Default view if token = 0
		if (vtok == 0)
			return default_view->used;

		// Retrieve the view from hash map otherwise
		RSHashMap_t::iterator it = state_views.find(vtok);
		if (it == state_views.end())
			return 0;
		return it->second->used;
	}

	/**
	 * @brief Resource available
	 * @param vtok The token referencing the resource view
	 * @return How much resource is still available.
	 */
	inline uint64_t Availability(RViewToken_t vtok = 0) {
		// Default view if token = 0
		if (vtok == 0)
			return (total - default_view->used);

		// Retrieve the view from hash map otherwise
		RSHashMap_t::iterator it = state_views.find(vtok);
		if (it != state_views.end())
			return (total - it->second->used);

		// If the view is not found, it means that nothing has been allocated.
		// Thus the availability value to return is the total amount of
		// resource
		return total;
	}

	/**
	 * @brief Acquire a given amount of resource
	 * @param amount How much resource is required
	 * @param app_ptr The application requiring the resource
	 * @param vtok The token referencing the resource view
	 * @return The amount of resource acquired if success, 0 otherwise.
	 */
	inline uint64_t Acquire(uint64_t amount, Application const * app_ptr,
			RViewToken_t vtok = 0) {

		ResourceStatePtr_t view;
		if (vtok == 0)
			// Default view if token = 0
			view = default_view;
		else {
			// Retrieve the view (or create a new one)
			RSHashMap_t::iterator it = state_views.find(vtok);
			if (it != state_views.end())
				view = it->second;
			else {
				view = ResourceStatePtr_t(new ResourceState());
				state_views[vtok] = view;
			}
		}
		// Try to set the new "used" value
		uint64_t fut_used = view->used + amount;
		if (fut_used > total)
			return 0;
		// Set new used value and application that requested the resource
		view->used = fut_used;
		view->apps[app_ptr->Pid()] = amount;
		return amount;
	}

	/**
	 * @brief Release the resource
	 *
	 * Release the specific amount of resource used by an application
	 *
	 * @param app_ptr The application releasing the resource
	 * @param vtok The token referencing the resource view
	 * @return The amount of resource released
	 */
	inline uint64_t Release(Application const * app_ptr,
			RViewToken_t vtok =	0) {

		ResourceStatePtr_t view;
		if (vtok == 0)
			// Default view if token = 0
			view = default_view;
		else {
			// Retrieve the view
			RSHashMap_t::iterator it = state_views.find(vtok);
			if (it == state_views.end())
				return 0;
			view = it->second;
		}
		// Lookup the application amount and subtract to used
		AppUseQtyMap_t::iterator lkp = view->apps.find(app_ptr->Pid());
		if (lkp == view->apps.end())
			return 0;
		view->used -= lkp->second;
		// Return the amount of resource released
		return lkp->second;
	}

	/**
	 * @brief Set a state view as default
	 *
	 * This call set the new default view to the one specified through the
	 * token. It is needed when, for example, the Scheduler/Optimizer has
	 * defined a new scheduling and resource allocation, by using a temporary
	 * view. The following method allows it to say that such temporary must
	 * become the new state of the system.
	 * Note that we are assuming that Barbeque (with ResourceAccounter) will
	 * consider the set of resource default view as its system resources state
	 * view.
	 *
	 * @param vtok The token of the view
	 * @return The token of the default view. Thus the new one if success, the
	 * old one otherwise.
	 */
	inline RViewToken_t SetAsDefaultView(RViewToken_t vtok) {
		// Retrieve the view
		RSHashMap_t::iterator it = state_views.find(vtok);
		if (it != state_views.end()) {
			// Set the new default view
			default_view = it->second;
			state_views.erase(default_key);
			default_key = vtok;
		}
		return default_key;
	}

	/**
	 * @brief Delete a state view
	 *
	 * If the token refers to the default view, the method returns doing
	 * nothing. This control is ahead of safety and consistency purposes.
	 * Indeed if the default view was removed, what state should be picked as
	 * the new default one ?
	 * Thus, this constraint force the caller to set a new default view, before
	 * delete the current one.
	 *
	 * @param vtok The token of the view to delete
	 */
	inline void DeleteView(RViewToken_t vtok) {
		if (vtok == default_key)
			return;
		state_views.erase(vtok);
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

	/** The key of the default state view */
	RViewToken_t default_key;

	/** Pointer to the default state view */
	ResourceStatePtr_t default_view;

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

