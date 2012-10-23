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

#include "bbque/system.h"
#include "bbque/app/application_conf.h"
#include "bbque/app/working_mode.h"
#include "bbque/res/resources.h"

// The prefix for logging statements category
#define SCHEDULER_POLICY_NAMESPACE "bq.sp"
// The prefix for configuration file attributes
#define SCHEDULER_POLICY_CONFIG "SchedPol"

using namespace bbque::app;
using namespace bbque::res;

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
	 * @brief The scheduling entity to evaluate
	 *
	 * A scheduling entity is characterized by the Application/EXC to schedule, a
	 * Working Mode, and a Cluster ID referencing the resource binding
	 */
	struct EvalEntity_t {

		/**
		 * @brief Constructor
		 *
		 * @param _papp Application/EXC to schedule
		 * @param _pawm AWM to evaluate
		 * @param _clid Cluster ID for resource binding
		 */
		EvalEntity_t(AppCPtr_t _papp, AwmPtr_t _pawm, uint8_t _clid):
			papp(_papp),
			pawm(_pawm),
			clust_id(_clid) {
			// Log string
			snprintf(str_id, 40, "[%s] {AWM:%02d,CL:%02d}", papp->StrId(),
					pawm->Id(), clust_id);
		};

		/** Application/EXC to schedule */
		AppCPtr_t papp;
		/** Candidate AWM */
		AwmPtr_t pawm;
		/** Candidate cluster for resource binding */
		ResID_t clust_id;
		/** Identifier string */
		char str_id[40];

		/** Return the identifier string */
		inline const char * StrId() const {
			return str_id;
		}
	};

	/**
	 * @brief Scheduling entity
	 *
	 * This embodies all the information needed in the "selection" step to require
	 * a scheduling for an application into a specific AWM, with the resource set
	 * bound into a chosen cluster
	 */
	struct SchedEntity_t: public EvalEntity_t {

		/**
		 * @brief Constructor
		 *
		 * @param _papp Application/EXC to schedule
		 * @param _pawm AWM to evaluate
		 * @param _clid Cluster ID for resource binding
		 * @param _metr The related scheduling metrics (also "application
		 * value")
		 */
		SchedEntity_t(AppCPtr_t _papp, AwmPtr_t _pawm, uint8_t _clid,
				float _metr):
			EvalEntity_t(_papp, _pawm, _clid),
			metrics(_metr) {
		};

		/** Metrics computed */
		float metrics;
	};

	/**
	 * @brief Return the name of the optimization policy
	 * @return The name of the optimization policy
	 */
	virtual char const * Name() = 0;

	/**
	 * @brief Schedule a new set of applciation on available resources.
	 *
	 * @param system a reference to the system interfaces for retrieving
	 * information related to both resources and applications.
	 * @param rvt a token representing the view on resource allocation, if
	 * the scheduling has been successfull.
	 */
	virtual ExitCode_t Schedule(bbque::System & system,
			bbque::res::RViewToken_t &rvt) = 0;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_SCHEDULER_POLICY_H_
