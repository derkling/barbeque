/**
 *       @file  synchronization_policy.h
 *      @brief  The interface for an applications synchronization policy
 *
 * This defines an abstract class for interaction between the BarbequeRTRM and
 * a policy for synchronizing applications status.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
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

#define SYNCHRONIZATION_POLICY_NAMESPACE "synp."

using bbque::app::ApplicationStatusIF;

namespace bbque { namespace plugins {

/**
 * @brief A module interface to implement resource scheduler policies.
 *
 * This class could be used to implement applications synchronization
 * alghoritms and heuristics.
 */
class SynchronizationPolicyIF {

public:

	/**
	 * @brief A synchronization latency [ms]
	 */
	typedef uint32_t SyncLatency_t;

	/**
	 * @brief Result codes reported by some of the methods of plugins
	 * implementing this interface
	 */
	typedef enum ExitCode {
		SYNCP_OK,
		SYNCP_ABORT_SYNC,
		SYNCP_FORCE_STOP
	} ExitCode_t;

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
	 * applicaiton returne, a condition is checked using the DoSync method.
	 *
	 * @param system a reference to the system view which exposes information
	 * related to both resources and applications.
	 * @param restart set true to reset the synchroniztion policy internal
	 * state machine, thus requesting to restart from scratch the synchronization
	 * of all the applications.
	 *
	 * @return The synchronization state to sync, or SYNC_NONE if no more
	 * applications require to be synched.
	 *
	 * @see DoSync
	 */
	virtual ApplicationStatusIF::SyncState_t GetApplicationsQueue(
			bbque::SystemView & system, bool restart = false) = 0;

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

	/**
	 * @brief Acknowledge a [ms] synch latency for the specified application
	 *
	 * In response to a PreChange message, the RTLib report ane estimation of
	 * the next synchronization point for the corresponding EXC. This method
	 * is used to validate the synchronization latency with respect to the
	 * synchronization and optimization strategy defined by the policy.
	 *
	 * @return SYNCP_OK if the asserted latency could be accepted, one of the
	 * other defined exit codes in case of errors. The exit code implicitely
	 * defineds the required action.
	 */
	virtual ExitCode_t CheckLatency(AppPtr_t papp, SyncLatency_t latency) = 0;

	/**
	 * @brief Report the estimated [ms] synchronization interval
	 *
	 * This method returns the estimated synchronization time, in milliseconds,
	 * which defines the time interval to wait before checking for a
	 * synchronization point of the EXC notified since the last call to the
	 * GetApplicationsQueue()
	 */
	virtual SyncLatency_t EstimatedSyncTime() = 0;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_SYNCHRONIZATION_POLICY_H_

