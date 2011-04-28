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

#include "bbque/app/application.h"

using bbque::app::Application;

namespace bbque { namespace res {

// Forward declaration
struct Resource;
struct ResourceUsage;
struct ResourceState;


/** Shared pointer to Resource descriptor */
typedef std::shared_ptr<Resource> ResourcePtr_t;

/** Shared pointer to ResourceState object */
typedef std::shared_ptr<ResourceState> ResourceStatePtr_t;

/** Shared pointer to ResourceUsage object */
typedef std::shared_ptr<ResourceUsage> UsagePtr_t;

/** List of Resource objects */
typedef std::list<ResourcePtr_t> ResourcePtrList_t;

/** Map of ResourceUsage objects. Key: resource path */
typedef std::map<std::string, UsagePtr_t> UsagesMap_t;

/** Map of Application descriptor pointers. Key: application name */
typedef std::map<std::string, Application const *> AppMap_t;

/**
 * Map of amounts of resource used by applications
 * Key: Application PID
 */
typedef std::map<uint32_t, uint64_t> AppUseMap_t;


/**
 * @enum Exit codes for Resource and ResourceState methods
 */
enum ExitCode_t {
	/** Success */
	RSRC_SUCCESS = 0,
	/** Resource usage overflow */
	RSRC_USAGE_OVERFLOW
};

/**
 * @class ResourceState
 *
 * The usage state of a system resource.
 * This are the basic information we need to track upon resources. More
 * specifically, up to now, we keep track of the total amount, the number of
 * used and available resources.
 */
class ResourceState {

public:

	/**
	 * @brief Construct
	 */
	ResourceState(uint64_t tot_amount):
		used(0),
		total(tot_amount) {
	}

	/**
	 * @brief Amount of resource used
	 */
	inline uint64_t Used() { return used; }

	/**
	 * @brief Set the amount of resource used. The value must be lesser than
	 * total availability.
	 * @return Return the value set if success, 0 otherwise
	 */
	inline ExitCode_t SetUsed(uint64_t use) {
		// If the value overflows the total, the set is unvalidate
		if (use <= total) {
			used = use;
			return RSRC_SUCCESS;
		}
		return RSRC_USAGE_OVERFLOW;
	}

	/**
	 * @brief The total amount of resource
	 * @return The total amount
	 */
	inline uint64_t Total() { return total; }

	/**
	 * @brief Set the total amount of resource
	 * @param tot The amount to set
	 */
	inline void SetTotal(uint64_t tot) { total = tot; }

	/**
	 * @brief How much resource is still available.
	 * @return The available amount
	 */
	inline uint64_t Availability() { return (total - used); }

protected:

	/** The amount of resource used in the system   */
	uint64_t used;

	/** The total amount of resource in the system  */
	uint64_t total;

};

/**
 * @class Resource
 *
 * The class managing the concept of "resource" in the RTRM keeping track of
 * the runtime status (availability, used, total), the resource name ("pe1",
 * "cluster2", "mem0" and the applications that are using the resource.
 */
class Resource: public ResourceState {

public:

	/**
	 * @brief Constructor
	 * @param _name Resource name
	 */
	Resource(std::string const & _name):
		ResourceState(0),
		name(_name) {
	}

	/**
	 * @brief Constructor
	 *
	 * @param res_path Resource path
	 * @param res_type Resource type
	 * @param res_amount The total amount of resource
	 */
	Resource(std::string const & res_path, uint64_t res_amount):
		ResourceState(res_amount) {

		// Extract the name from the path
		uint16_t pos = res_path.find_last_of(".");
		if (pos > 0)
			name = res_path.substr(pos + 1);
		else
			name = res_path;
	}

	/**
	 * @brief Resource name
	 * @return The resource name string
	 */
	inline std::string const & Name() { return name; }

	/**
	 * @brief Acquire a given amount of resource
	 * @param amount How much resource is required
	 * @param app_ptr The application requiring the resource
	 * @return An exit code (@see ExitCode_t)
	 */
	inline ExitCode_t Acquire(uint64_t amount, ba::Application const * app_ptr) {
		// Set the new "used" value
		ExitCode_t ret = SetUsed(used + amount);
		// Keep track of the amount of resource used by this application
		if (ret == RSRC_SUCCESS)
			apps[app_ptr->Pid()] = amount;
		return ret;
	}

	/**
	 * @brief Release the resource
	 *
	 * Release the specific amount of resource used by an application
	 *
	 * @param app_ptr The application releasing the resource
	 */
	inline void Release(ba::Application const * app_ptr) {
		// Lookup the application amount
		AppUseMap_t::iterator lkp = apps.find(app_ptr->Pid());
		// Subtract the amount used
		if (lkp != apps.end())
			used -= lkp->second;
	}

private:

	/** Resource name (i.e. "mem0", "pe1", "dma1", ...)        */
	std::string name;

	/**
	 * Amounts of resource used by each of the applications holding the
	 * resource
	 */
	AppUseMap_t apps;

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
 */
struct ResourceUsage {

	/**
	 * Constructor
	 */
	ResourceUsage(uint64_t usage_value):
		value(usage_value) {
	}

	/** Usage value request */
	uint64_t value;

	/** List of resource descriptors which to the resource usage is bind*/
	ResourcePtrList_t binds;

};

}   // namespace res

}   // namespace bbque

#endif // BBQUE_RESOURCES_H_

