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


/** SystemView successfully instanced */
#define SV_SUCCESS 		0
/** Error: Application Manager missing */
#define SV_ERR_APP_MAN 	1
/** Error: Resource Accounter missing */
#define SV_ERR_RES_ACC 	2
/** Error: Missing both components */
#define SV_ERR_BOTH		3

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
	static SystemView *GetInstance() {
		static SystemView *instance;
		if (!instance) {
			// Construct a new SystemView instance
			uint8_t init_status;
			instance = new SystemView(init_status);
			if (init_status != SV_SUCCESS)
				delete instance;
		}
		return instance;
	}

	/**
	 * @brief Return the map containing all the ready applications
	 */
	inline AppsMap_t const * ApplicationsReady() {
		return app_man->Applications(app::ApplicationStatusIF::READY);
	}

	/**
	 * @brief Map of application (descriptors) just scheduled
	 */
	inline AppsMap_t const * ApplicationsScheduled() {
		return app_man->Applications(app::ApplicationStatusIF::SCHEDULED);
	}

	/**
	 * @brief Map of running applications (descriptors)
	 */
	inline AppsMap_t const * ApplicationsRunning() {
		return app_man->Applications(app::ApplicationStatusIF::RUNNING);
	}

	/**
	 * @brief Maximum integer value for the minimum application priority
	 */
	inline uint16_t ApplicationLowestPriority() const {
		return app_man->LowestPriority();
	}

	/**
	 * @brief The amount of resource available given an identifying resource
	 * path
	 * @param path Resource path
	 * @return The amount of resource available
	 */
	inline uint64_t ResourceAvailability(std::string const & path) const {
		return res_acc->Available(path);
	}

	/**
	 * @brief The total amount of resource given an Identifying resource path
	 * @param path Resource path
	 * @return The total amount of resource available
	 */
	inline uint64_t ResourceTotal(std::string const & path) const {
		return res_acc->Total(path);
	}

	/**
	 * @brief The used amount of resource given an identifying resource path
	 * @param path Resource path
	 * @return The used amount of resource available
	 */
	inline uint64_t ResourceUsed(std::string const & path) const {
		return res_acc->Used(path);
	}

	/**
	 * @brief Retrieve a pointer to the resource descriptor
	 * @param path Resource path
	 */
	inline res::ResourcePtr_t GetResource(std::string const & path) const {
		return res_acc->GetResource(path);
	}

	/**
	 * @brief Check the existence of a resource, looking for at least one
	 * compatible matching.
	 * @param path Resource path
	 * @return True if the resource exists, false otherwise.
	 */
	inline bool ExistResource(std::string const & path) const {
		return res_acc->ExistResource(path);
	}

private:

	/** ApplicationManager instance */
	ApplicationManagerStatusIF * app_man;

	/** ResourceAccounter instance */
	res::ResourceAccounterStatusIF * res_acc;

	/** Constructor */
	SystemView(uint8_t & status) {
		status = SV_SUCCESS;

		// Application Manager instance
		app_man = (ApplicationManagerStatusIF *)
			ApplicationManager::GetInstance();
		if (app_man == NULL)
			status += SV_ERR_APP_MAN;

		// Resource Accounter instance
		res_acc = (res::ResourceAccounterStatusIF *)
			res::ResourceAccounter::GetInstance();
		if (res_acc == NULL)
			status += SV_ERR_RES_ACC;
	}
};


} // namespace bbque

#endif  // BBQUE_SYSTEM_VIEW_H_

