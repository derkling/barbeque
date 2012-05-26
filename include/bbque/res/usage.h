/*
 * Copyright (C) 2012  Politecnico di Milano
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BBQUE_RESOURCE_USAGE_H_
#define BBQUE_RESOURCE_USAGE_H_

#include <cstdint>
#include <cstring>
#include <list>
#include <map>
#include <memory>
#include <string>

#include "bbque/res/resources.h"

namespace bbque { namespace res {

// Forward declaration
class Usage;

/** Shared pointer to Usage object */
typedef std::shared_ptr<Usage> UsagePtr_t;
/** Map of Usage descriptors. Key: resource path */
typedef std::map<std::string, UsagePtr_t> UsagesMap_t;
/** Constant pointer to the map of Usage descriptors */
typedef std::shared_ptr<UsagesMap_t> UsagesMapPtr_t;


/**
 * @brief The usages of a resource
 *
 * An application working modes defines a set of this resource requests
 * (usages).
 *
 * A resource usage descriptor embeds a couple of information:
 * The first is obviously the value of the usage, the amount of resource
 * requested. The second is a list containing all the descriptors (shared
 * pointer) of the resources which to this usage refers. We expect that such
 * list is filled by a method of ResourceAccounter, after that the
 * Scheduler/Optimizer has solved the resource binding.
 *
 * The "resource binding" can be explained as follows: if a Working Mode
 * includes resource requests like "tile.cluster2.pe = 4", the
 * scheduler/optimizer must define a binding between that "cluster2" and the
 * "real" cluster on the platform, to which bind the usage of 4 processing
 * elements. Thus, after that, the list "bindings", will contain the
 * descriptors of the resources "pe (processing elements)" in the cluster
 * assigned by the scheduler/optimizer module.
 */
class Usage {

friend class bbque::ResourceAccounter;

public:

	/**
	 * @enum ExitCode_t
	 */
	enum ExitCode_t {
		/** Success */
		RU_OK = 0,
		/** Application pointer is null */
		RU_ERR_NULL_POINTER,
		/** Application pointer mismatch */
		RU_ERR_APP_MISMATCH,
		/** Resource state view token mismatch */
		RU_ERR_VIEW_MISMATCH
	};

	/**
	 * @brief Constructor
	 * @param usage_value The amount of resource usage
	 */
	Usage(uint64_t usage_value);

	/**
	 * @brief Destructor
	 */
	~Usage();

	/**
	 * @brief The amount of resource required/assigned to the Application/EXC
	 *
	 * @return The amount of resource
	 */
	uint64_t GetAmount();

	/**
	 * @brief Get the entire list of resource bindings
	 *
	 * @return A reference to the resource bindings list
	 */
	ResourcePtrList_t & GetBindingList();

	/**
	 * @brief Set the list of resource bindings
	 *
	 * Commonly a Usage object specifies the request of a specific
	 * type of resource, which can be bound on a set of platform resources
	 * (i.e. "tile0,cluster2.pe" -> "...cluster2.pe{0|1|2|...}".
	 * The bindings list includes the pointers to all the resource descriptors
	 * that can satisfy the request. The method initialises the iterators
	 * pointing to the set of resource bindings effectively granted to the
	 * Application/EXC.
	 *
	 * @param bind_list The list of resource descriptor for binding
	 */
	void SetBindingList(ResourcePtrList_t bind_list);

	/**
	 * @brief Check of the resource binding list is empty
	 *
	 * @return true if the list is empty, false otherwise.
	 */
	bool EmptyBindingList();

	/**
	 * @brief Get the first resource from the binding list
	 *
	 * @param it The list iterator. This is set and hence must be used for
	 * iteration purposes.
	 *
	 * @return The pointer (shared) to the resource descriptor providing a
	 * first quota (or the whole of it) of the resource usage required.
	 */
	ResourcePtr_t GetFirstResource(ResourcePtrListIterator_t & it);

	/**
	 * @brief Get the last resource from the binding list
	 *
	 * @param it The list iterator returned by GetFirstResource() or a
	 * previous call to GetNextResource().
	 *
	 * @return The pointer (shared) to the resource descriptor providing a
	 * quota of the resource usage required.
	 */
	ResourcePtr_t GetNextResource(ResourcePtrListIterator_t & it);

	/**
	 * @brief Track the first resource, from the binding list, granted to the
	 * Application/EXC
	 *
	 * The binding list tracks the whole set of possible resources to which
	 * bind the usage request. This method set the first resource granted to
	 * the Application/EXC, to satisfy part of the usage request, or the whole
	 * of it.
	 *
	 * @param papp The Application/EXC requiring the resource
	 * @param first_it The list iterator of the resource binding to track
	 * @param vtok The token of the resource state view into which the
	 * assignment has been performed.
	 *
	 * @return RU_OK if success. RU_ERR_NULL_POINTER if the pointer to the
	 * application is null.
	 */
	ExitCode_t TrackFirstBinding(AppSPtr_t const & papp,
			ResourcePtrListIterator_t & first_it, RViewToken_t vtok);

	/**
	 * @brief Track the last resource, from the binding list, granted to the
	 * Application/EXC
	 *
	 * The binding list tracks the whole set of possible resources to which
	 * bind the usage request. This method set the last resource granted to
	 * the Application/EXC, to satisfy the last quota of the usage request.
	 *
	 * @param papp The Application/EXC requiring the resource
	 * @param last_it The list iterator of the resource binding to track
	 * @param vtok The token of the resource state view into which the
	 * assignment has been performed.
	 *
	 * @return RU_OK if success.
	 * RU_ERR_NULL_POINTER if the pointer to the application is null.
	 * RU_ERR_APP_MISMATCH if the application specified differs from the one
	 * specified in TrackFirstBinding().
	 * RU_ERR_VIEW_MISMATCH if the state view token does not match the one set
	 * in TrackFirstBinding().
	 */
	ExitCode_t TrackLastBinding(AppSPtr_t const & papp,
			ResourcePtrListIterator_t & last_it, RViewToken_t vtok);

private:

	/** Usage value request */
	uint64_t value;

	/** List of resource descriptors which to the resource usage is bound */
	ResourcePtrList_t bindings;

	/** The application/EXC owning this resource usage */
	AppSPtr_t own_app;

	/** The token referencing the state view of the resource usage */
	RViewToken_t view_tk;

	/** List iterator pointing to the first resource used by the App/EXC */
	ResourcePtrListIterator_t first_bind;

	/** List iterator pointing to the last resource used by the App/EXC */
	ResourcePtrListIterator_t last_bind;
};

} // namespace res

} // namespace bbque

#endif // BBQUE_RESOURCE_USAGE_H
