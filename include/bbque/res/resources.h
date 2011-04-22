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

/** Map of ResourceUsage objects. Key: resource path */
typedef std::map<std::string, UsagePtr_t> UsagesMap_t;

/** Map of Application descriptor pointers. Key: application name */
typedef std::map<std::string, Application const *> AppMap_t;


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
 * The usage state of a system resource. We just keep track of the total
 * amount of a given resource, and its usage level.
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
	 * @brief Increment the amount of resource used by adding an integer value
	 * @param add The addend value
	 */
	inline ExitCode_t AddUsed(uint64_t add) {
		return SetUsed(used + add);
	}

	/**
	 * @brief Decrement the amount of resource used by subtracting an integer
	 * value
	 * @param sub The subtracting value
	 */
	inline ExitCode_t SubUsed(uint64_t sub) {
		return SetUsed(used - sub);
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
 * the runtime status (availability, used, total) and other information such
 * as the type of resource (i.e. "memory", "cpu, "bus"...) and the
 * applications that are using the resource.
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
	Resource(std::string const & res_path, std::string const & res_type,
			uint64_t res_amount):
		ResourceState(res_amount),
		type(res_type) {

		// Extract the name from the path
		uint16_t pos = res_path.find_last_of(".");
		if (pos > 0)
			name = res_path.substr(pos + 1);
		else
			name = res_path;
	}

	/**
	 * @brief Resource name
	 */
	inline std::string const & Name() { return name; }

	/**
	 * @brief Resource Type(i.e. "cpu", "memory","IO", ...)
	 */
	inline std::string const & Type() { return type; }

	/**
	 * @brief Set the resource type
	 * @param res_type Resource type (i.e "processor", "memory", ...)
	 */
	inline void SetType(std::string const & res_type) {
		type = res_type;
	}

	/**
	 * @brief Set the resource as used by a given application
	 * @param app_ptr The pointer to the application using the resource
	 */
	inline void UsedBy(Application const * app_ptr) {
		apps[app_ptr->Name()] = app_ptr;
	}

	/**
	 * @brief Unset the the application from the use of the resource
	 * @param app_ptr The pointer to the application pastly using the resource
	 */
	inline void NoMoreUsedBy(Application const * app_ptr) {
		apps.erase(app_ptr->Name());
	}

private:

	/** Resource name (i.e. "mem0", "pe1", "dma1", ...)        */
	std::string name;

	/** Resource type (i.e. "processor", "memory", "bandwidth",...)  */
	std::string type;

	/** Applications holding the resource     */
	AppMap_t apps;

};

/**
 * @struct ResourceUsage
 *
 * An application working modes defines a set of this resource requests
 * (usages)
 */
struct ResourceUsage {

	/** Pointer to the resource in use    */
	ResourcePtr_t resource;

	/** Usage value */
	uint64_t value;

};

}   // namespace res

}   // namespace bbque

#endif // BBQUE_RESOURCES_H_

