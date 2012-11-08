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

#ifndef BBQUE_SYSTEM_H_
#define BBQUE_SYSTEM_H_

#include "bbque/application_manager.h"
#include "bbque/resource_accounter.h"

using bbque::app::ApplicationStatusIF;
using bbque::app::AppCPtr_t;
using bbque::res::ResourcePtr_t;
using bbque::res::ResourcePtrList_t;

namespace bbque {

/**
 * @brief A unified view on system status
 *
 * This class provides all the methods necessary to get an aggregated view of
 * the system status, where under "system" we put the set of applications and
 * resources managed by the RTRM.  When instanced, the class get references to
 * the ApplicationManager and ResourceAccounter instances and it provides a
 * simplified set of methods for making queries upon applications and
 * resources status.
 */
class System {

public:

	/**
	 * @brief Get the SystemVIew instance
	 */
	static System & GetInstance() {
		static System instance;
		return instance;
	}

	/// ...........................: APPLICATIONS :...........................

	/**
	 * @brief Return the first app at the specified priority
	 */
	inline AppCPtr_t GetFirstWithPrio(AppPrio_t prio, AppsUidMapIt & ait) {
		return am.GetFirst(prio, ait);
	}

	/**
	 * @brief Return the next app at the specified priority
	 */
	inline AppCPtr_t GetNextWithPrio(AppPrio_t prio, AppsUidMapIt & ait) {
		return am.GetNext(prio, ait);
	}


	/**
	 * @brief Return the map containing all the ready applications
	 */
	inline AppCPtr_t GetFirstReady(AppsUidMapIt & ait) {
		return am.GetFirst(ApplicationStatusIF::READY, ait);
	}

	inline AppCPtr_t GetNextReady(AppsUidMapIt & ait) {
		return am.GetNext(ApplicationStatusIF::READY, ait);
	}

	/**
	 * @brief Map of running applications (descriptors)
	 */
	inline AppCPtr_t GetFirstRunning(AppsUidMapIt & ait) {
		return am.GetFirst(ApplicationStatusIF::RUNNING, ait);
	}

	inline AppCPtr_t GetNextRunning(AppsUidMapIt & ait) {
		return am.GetNext(ApplicationStatusIF::RUNNING, ait);
	}

	/**
	 * @brief Map of blocked applications (descriptors)
	 */

	inline AppCPtr_t GetFirstBlocked(AppsUidMapIt & ait) {
		return am.GetFirst(ApplicationStatusIF::BLOCKED, ait);
	}

	inline AppCPtr_t GetNextBlocked(AppsUidMapIt & ait) {
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
	 * @see ApplicationManagerStatusIF
	 */
	inline uint16_t ApplicationsCount(AppPrio_t prio) {
		return am.AppsCount(prio);
	}

	/**
	 * @see ApplicationManagerStatusIF
	 */
	inline uint16_t ApplicationsCount(ApplicationStatusIF::State_t state) {
		return am.AppsCount(state);
	}

	/**
	 * @see ApplicationManagerStatusIF
	 */
	inline uint16_t ApplicationsCount(ApplicationStatusIF::SyncState_t state) {
		return am.AppsCount(state);
	}

	/**
	 * @brief Maximum integer value for the minimum application priority
	 */
	inline uint16_t ApplicationLowestPriority() const {
		return am.LowestPriority();
	}


	/// .............................: RESOURCES :............................

	/**
	 * @see ResourceAccounterStatusIF::Available()
	 */
	inline uint64_t ResourceAvailable(std::string const & path,
			RViewToken_t vtok = 0, AppCPtr_t papp = AppCPtr_t()) const {
		return ra.Available(path, vtok, papp);
	}

	/**
	 * @see ResourceAccounterStatusIF::Available()
	 */
	inline uint64_t ResourceAvailable(ResourcePtrList_t & rsrc_list,
			RViewToken_t vtok = 0, AppCPtr_t papp = AppCPtr_t()) const {
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
	inline uint64_t ResourceUsed(std::string const & path,
			RViewToken_t vtok = 0) const {
		return ra.Used(path, vtok);
	}

	/**
	 * @see ResourceAccounterStatusIF::Used()
	 */
	inline uint64_t ResourceUsed(ResourcePtrList_t & rsrc_list,
			RViewToken_t vtok = 0) const {
		return ra.Used(rsrc_list, vtok);
	}

	/**
	 * @see ResourceAccounterStatusIF::Count()
	 */
	inline uint32_t ResourceCount(std::string const & path) const;
	 * @see ResourceAccounterStatusIF::GetResource()
	 */
	inline ResourcePtr_t GetResource(std::string const & path) const {
		return ra.GetResource(path);
	}

	/**
	 * @see ResourceAccounterStatusIF::GetResources()
	 */
	inline ResourcePtrList_t GetResources(std::string const & temp_path)
		const {
		return ra.GetResources(temp_path);
	}

	/**
	 * @see ResourceAccounterStatusIF::ExistResources()
	 */
	inline bool ExistResource(std::string const & path) const {
		return ra.ExistResource(path);
	}

	/**
	 * @see ResourceAccounterStatusIF::GetNumResources()
	 */
	inline uint16_t GetNumResources(std::string const & type) const {
		return ra.GetNumResources(type);
	}

	/**
	 * @see ResourceAccounterStatusIF::GetNumResourceTypes()
	 */
	inline uint16_t GetNumResourceTypes() const {
		return ra.GetNumResourceTypes();
	}

	/**
	 * @see ResourceAccounterConfIF::GetView()
	 */
	inline ResourceAccounterStatusIF::ExitCode_t GetResourceStateView(
			std::string req_id, RViewToken_t & tok) {
		return ra.GetView(req_id, tok);
	}

	/**
	 * @see ResourceAccounterConfIF::PutView()
	 */
	inline void PutResourceStateView(RViewToken_t tok) {
		return ra.PutView(tok);
	}

private:

	/** ApplicationManager instance */
	ApplicationManagerStatusIF & am;

	/** ResourceAccounter instance */
	ResourceAccounterConfIF & ra;

	/** Constructor */
	System() :
		am(ApplicationManager::GetInstance()),
		ra(ResourceAccounter::GetInstance()) {
	}
};


} // namespace bbque

#endif  // BBQUE_SYSTEM_H_
