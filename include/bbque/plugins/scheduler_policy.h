/**
 *       @file  scheduler_policy.h
 *      @brief  The interface for channel to communicate with applications
 *
 * This defines the common interface for interaction between Barbeque and
 * applications.  This interface defines also the calls to implement a
 * three-way Synchronization Protocol which is used to sync the RTRM status
 * with the effective applications status.
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

#ifndef BBQUE_SCHEDULER_POLICY_H_
#define BBQUE_SCHEDULER_POLICY_H_

#include "bbque/res/resources.h"

#define SCHEDULER_POLICY_NAMESPACE "schp."

namespace bbque {
	class SystemView;
}

namespace bbque { namespace plugins {

/**
 * @class SchedulerPolicyIF
 * @brief A module interface to implement resource scheduler policies.
 *
 * This class could be used to implement resource scheduling alghoritms and
 * heuristics.
 */
class SchedulerPolicyIF {

public:

	/**
	 * @enum ExitCode_t
	 * @brief Scheduling result
	 */
	typedef enum ExitCode {
		/** Scheduling done */
		SCHED_DONE = 0,
		/** Successful return */
		SCHED_OK,
		/** Resource availability */
		SCHED_RSRC_UNAV,
		/** No more PEs available in the cluster */
		SCHED_CLUSTER_FULL,
		/** Application must be skipped due to a Disable/Stop event */
		SCHED_SKIP_APP,
		/** Error */
		SCHED_ERROR
	} ExitCode_t;

	/**
	 * @brief Return the name of the optimization policy
	 * @return The name of the optimization policy
	 */
	virtual char const * Name() = 0;

	/**
	 * @brief Schedule a new set of applciation on available resources.
	 *
	 * @param system a reference to the system view which exposes information
	 * related to both resources and applications.
	 * @param rvt a token representing the view on resource allocation, if
	 * the scheduling has been successfull.
	 */
	virtual ExitCode_t Schedule(bbque::SystemView & system,
			bbque::res::RViewToken_t &rvt) = 0;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_SCHEDULER_POLICY_H_

