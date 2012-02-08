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

#ifndef BBQUE_SCHEDULER_POLICY_H_
#define BBQUE_SCHEDULER_POLICY_H_

#include "bbque/res/resources.h"

#define SCHEDULER_POLICY_NAMESPACE "bq.sp."

namespace bbque {
	class SystemView;
}

namespace bbque { namespace plugins {

/**
 * @brief A module interface to implement resource scheduler policies.
 * @ingroup sec05_sm
 *
 * This is an abstract class for interaction between the BarbequeRTRM and
 * a policy for scheduling of available resources.
 * This class could be used to implement resource scheduling alghoritms and
 * heuristics.
 */
class SchedulerPolicyIF {

public:

	/**
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

/*******************************************************************************
 *    Doxygen Module Documentation
 ******************************************************************************/

/**
 * @ingroup sec05_sm
 *
 * The SchedulerPolicyIF is an abstract class which defines the common
 * interface for interaction between the BarbequeRTRM and a resource
 * scheduling policy.
 *
 * Such a policy is used by the SchedulerManager (SM) to setup a new
 * "resource view", identified by a RViewToken_t, where available resources
 * are partitioned and assigned to demanding applications based on a certain
 * "optimization stragery".
 *
 * ADD MORE DETAILS HERE
 */

#endif // BBQUE_SCHEDULER_POLICY_H_
