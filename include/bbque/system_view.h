/**
 *       @file  system_view.h
 *      @brief  System view interface
 *
 * This provides the interface to query applications and resources runtime
 * status accordingly to the interfaces exposed by ApplicationManager and
 * ResourceAccounter components.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  06/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_SYSTEM_VIEW_H_
#define BBQUE_SYSTEM_VIEW_H_

#include "bbque/application_manager.h"
#include "bbque/res/resource_accounter.h"

using bbque::ApplicationManager;
using bbque::ApplicationManagerStatusIF;
using bbque::app::ApplicationStatusIF;
using bbque::res::ResourceAccounter;
using bbque::res::ResourceAccounterStatusIF;

namespace bbque {

/**
 * @class SystemView
 *
 * This is as an aggregated view of the system status, where under "system" we
 * put the set of applications and resources managed by the RTRM.
 * When instanced, the class get references to the ApplicationManager and
 * ResourceAccounter instances and it provides a set of methods for making
 * queries upon applications and resources status.
 */
class SystemView {

public:

	/**
	 * @brief Get the SystemVIew instance
	 */
	static SystemView & GetInstance() {
		static SystemView instance;
		return instance;
	}

	/**
	 * @brief Return the map containing all the ready applications
	 */
	}

	/**
	 * @brief Map of running applications (descriptors)
	 */
	}

	/**
	 * @brief Map of blocked applications (descriptors)
	 */
	}

	/**
	 * @see ApplicationManagerStatusIF
	 */
	inline bool HasApplications(AppPrio_t prio) {
		return am.HasApplications(prio);
	}

	/**
	 * @see ApplicationManagerStatusIF
	 */
	inline bool HasApplications(ApplicationStatusIF::State_t state) {
		return am.HasApplications(state);
	}

	/**
	 * @see ApplicationManagerStatusIF
	 */
	inline bool HasApplications(ApplicationStatusIF::SyncState_t sync_state) {
		return am.HasApplications(sync_state);
	}

	/**
	 * @brief Maximum integer value for the minimum application priority
	 */
	inline uint16_t ApplicationLowestPriority() const {
		return am.LowestPriority();
	}

	/**
	 * @brief The amount of resource available given an identifying resource
	 * path
	 * @param path Resource path
	 * @return The amount of resource available
	 */
	inline uint64_t ResourceAvailability(std::string const & path) const {
		return ra.Available(path);
	}

	/**
	 * @brief The total amount of resource given an Identifying resource path
	 * @param path Resource path
	 * @return The total amount of resource available
	 */
	inline uint64_t ResourceTotal(std::string const & path) const {
		return ra.Total(path);
	}

	/**
	 * @brief The used amount of resource given an identifying resource path
	 * @param path Resource path
	 * @return The used amount of resource available
	 */
	inline uint64_t ResourceUsed(std::string const & path) const {
		return ra.Used(path);
	}

	/**
	 * @brief Retrieve a pointer to the resource descriptor
	 * @param path Resource path
	 * @return A shared pointer to the resource desciptor found
	 */
	inline res::ResourcePtr_t GetResource(std::string const & path) const {
		return ra.GetResource(path);
	}

	/**
	 * @brief List of resource descriptors matching a template path
	 * @param path Resource template path
	 * @return A list of resource descriptors
	 */
	inline res::ResourcePtrList_t
	GetResources(std::string const & temp_path) const {
			return ra.GetResources(temp_path);
	}

	/**
	 * @brief Check the existence of a resource, looking for at least one
	 * compatible matching.
	 * @param path Resource path
	 * @return True if the resource exists, false otherwise.
	 */
	inline bool ExistResource(std::string const & path) const {
		return ra.ExistResource(path);
	}

	/**
	 * @brief Resource clustering factor
	 * @see ClusteringFactor
	 */
	inline uint16_t ResourceClusterFactor(std::string const & path) {
		return ra.ClusteringFactor(path);
	}

private:

	/** ApplicationManager instance */
	ApplicationManagerStatusIF & am;

	/** ResourceAccounter instance */
	ResourceAccounterStatusIF & ra;

	/** Constructor */
	SystemView() :
		am(ApplicationManager::GetInstance()),
		ra(ResourceAccounter::GetInstance()) {
	}
};


} // namespace bbque

#endif  // BBQUE_SYSTEM_VIEW_H_

