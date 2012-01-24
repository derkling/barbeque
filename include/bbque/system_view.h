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
	 * @brief Return the first app at the specified priority
	 */
	inline AppPtr_t GetFirstWithPrio(AppPrio_t prio, AppsUidMapIt & ait) {
		return am.GetFirst(prio, ait);
	}

	/**
	 * @brief Return the next app at the specified priority
	 */
	inline AppPtr_t GetNextWithPrio(AppPrio_t prio, AppsUidMapIt & ait) {
		return am.GetNext(prio, ait);
	}


	/**
	 * @brief Return the map containing all the ready applications
	 */
	inline AppPtr_t GetFirstReady(AppsUidMapIt & ait) {
		return am.GetFirst(ApplicationStatusIF::READY, ait);
	}

	inline AppPtr_t GetNextReady(AppsUidMapIt & ait) {
		return am.GetNext(ApplicationStatusIF::READY, ait);
	}

	/**
	 * @brief Map of running applications (descriptors)
	 */
	inline AppPtr_t GetFirstRunning(AppsUidMapIt & ait) {
		return am.GetFirst(ApplicationStatusIF::RUNNING, ait);
	}

	inline AppPtr_t GetNextRunning(AppsUidMapIt & ait) {
		return am.GetNext(ApplicationStatusIF::RUNNING, ait);
	}

	/**
	 * @brief Map of blocked applications (descriptors)
	 */

	inline AppPtr_t GetFirstBlocked(AppsUidMapIt & ait) {
		return am.GetFirst(ApplicationStatusIF::BLOCKED, ait);
	}

	inline AppPtr_t GetNextBlocked(AppsUidMapIt & ait) {
		return am.GetNext(ApplicationStatusIF::BLOCKED, ait);
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
	 * @see ResourceAccounterStatusIF::Available()
	 */
	inline uint64_t ResourceAvailable(std::string const & path,
			RViewToken_t vtok = 0, AppPtr_t papp = AppPtr_t()) const {
		return ra.Available(path, vtok, papp);
	}

	/**
	 * @see ResourceAccounterStatusIF::Available()
	 */
	inline uint64_t ResourceAvailable(ResourcePtrList_t & rsrc_list,
			RViewToken_t vtok = 0, AppPtr_t papp = AppPtr_t()) const {
		return ra.Available(rsrc_list, vtok, papp);
	}

	/**
	 * @see ResourceAccounterStatusIF::Total()
	 */
	inline uint64_t ResourceTotal(std::string const & path) const {
		return ra.Total(path);
	}

	/**
	 * @see ResourceAccounterStatusIF::Total()
	 */
	inline uint64_t ResourceTotal(ResourcePtrList_t & rsrc_list) const {
		return ra.Total(rsrc_list);
	}

	/**
	 * @see ResourceAccounterStatusIF::Used()
	 */
	inline uint64_t ResourceUsed(std::string const & path) const {
		return ra.Used(path);
	}

	/**
	 * @see ResourceAccounterStatusIF::Used()
	 */
	inline uint64_t ResourceUsed(ResourcePtrList_t & rsrc_list) const {
		return ra.Used(rsrc_list);
	}

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

