/**
 *       @file  synchronization_policy.h
 *      @brief  The interface provided by a Synchronization Policy plugin
 *
 * This defines the interface which should be supported by synchronization
 * policy plugins. Such a policy is used by the SynchronizationManager (SM) to
 * identify the set of EXCs which should be synchronized. The policy should
 * just select the EXCs and than pass them back to the SM to perform the
 * actual synchronization.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#ifndef BBQUE_SYNCHRONIZATION_POLICY_H_
#define BBQUE_SYNCHRONIZATION_POLICY_H_

#include "bbque/application_manager.h"
#include "bbque/app/application.h"
#include "bbque/system_view.h"

#define SYNCHRONIZATION_POLICY_NAMESPACE "sync.pol."

namespace bbque { namespace plugins {

/**
 * @class SynchronizationPolicyIF
 * @brief A module interface to implement resource scheduler policies.
 *
 * This class could be used to implement resource scheduling alghoritms and
 * heuristics.
 */
class SynchronizationPolicyIF {

public:

	/**
	 * @brief Return the name of the synchronization policy
	 * @return The name of the synchronization policy
	 */
	virtual char const * Name() = 0;

	/**
	 * @brief Get a new map of applications to synchronize.
	 *
	 * The SynchronizationManager (SM) delegate the selection of applications
	 * to be synched to a policy defined by plugins implementing this
	 * interface.  More precisely, the (SM) issues a set of calls to thus
	 * method of a policy to know which applications sould be synchronized.
	 * This method allows a synchronization policy to implement a custom
	 * selection of applications to be synchronized, by ordering them
	 * according to its internal decisions. Indeed, the SM component provides
	 * just the raw mechanisms to carry on the synchronization of a pull of
	 * selected applications: those returned by this call.  Moreover, for each
	 * applicaiton returne, a condition is checked using the DoSyncmethod.
	 *
	 * @param system a reference to the system view which exposes information
	 * related to both resources and applications.
	 * @params restart set true to reset the synchroniztion policy internal
	 * state machine, thus requesting to restart from scratch the synchronization
	 * of all the applications.
	 *
	 * @return A map of applications to sync, an empty map if not more
	 * applications require to be synched.
	 *
	 * @see DoSync
	 */
	virtual bbque::AppsUidMap_t const * GetApplicationsQueue(
			bbque::SystemView const & system, bool restart = false) = 0;

	/**
	 * @brief Check if the application should be synched
	 *
	 * This method allows to verify if the specified application should be
	 * synched at the time of the call. Such a method allows to define quite
	 * generic synchronization policy where a bounch of applications is
	 * selected by the GetApplicationsQueue and then, within this set, only
	 * some are actually synchronized.
	 *
	 * @return true if the specified application should be synchronized, false
	 * otherwise.
	 *
	 * @note this call is on the synchronization critical path, thus the
	 * verified condition should (eventually) introduce a very low overhead.
	 */
	virtual bool DoSync(AppPtr_t papp) = 0;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_SYNCHRONIZATION_POLICY_H_

