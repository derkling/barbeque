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

#ifndef BBQUE_SC_FAIRNESS_
#define BBQUE_SC_FAIRNESS_

#include "sched_contrib.h"

#define DEFAULT_CONG_EXPBASE 	2

using bbque::app::AppPrio_t;

namespace bbque { namespace plugins {


class SCFairness: public SchedContrib {

public:

	/**
	 * @brief Constructor
	 *
	 * @param name A name identifying the specific contribute
	 * @param cfg_params Global configuration parameters
	 */
	SCFairness(const char * _name, uint16_t const cfg_params[]);

	/**
	 * @brief Perform per-priority class information setup
	 *
	 * Get the number of applications in the given priority level, the
	 * availability (not clustered), and computes the fair partitions of each
	 * resource type
	 *
	 * @param params Expected pointer to AppPrio_t type
	 *
	 * @return SC_SUCCESS. No error conditions expected
	 */
	ExitCode_t Init(void * params);

private:

	/** Base for exponential functions used in the computation */
	uint16_t expbase;
	/**
	 * Congestion penalties per resource type. This stores the values parsed
	 * from the configuration file.
	 *
	 * 0 = "pe"
	 * 1 = "mem"
	 */
	uint16_t penalties_int[SC_RSRC_COUNT];

	/** Default values for the congestion penalties */
	static uint16_t penalties_default[SC_RSRC_COUNT];


	/** Number of applications to schedule */
	uint16_t num_apps;

	/** Resource availability */
	uint64_t rsrc_avail[SC_RSRC_COUNT];

	/** Fair partitions */
	uint64_t fair_parts[SC_RSRC_COUNT];


	/**
	 * @brief Compute the congestion contribute
	 *
	 * @param evl_ent The entity to evaluate (EXC/AWM/ClusterID)
	 * @param ctrib The contribute to set
	 *
	 * @return SC_SUCCESS for success
	 */
	ExitCode_t _Compute(SchedulerPolicyIF::EvalEntity_t const & evl_ent,
			float & ctrib);

	/**
	 * @brief Set the parameters for the filter function
	 *
	 * More in detail the parameters set are exclusively the ones of the
	 * exponential function, since the Sub Fair Region (SFR) returns the
	 * constant index (1) and the Over Fair Region (OFR) the result of the
	 * exponential function.
	 *
	 * @param cfp Cluster fair partition. Used to compute the "x-scale"
	 * parameter.
	 * @param cra Cluster resource availability. This is the "x-offset" and a
	 * component of the "x-scale" parameter.
	 * @param penalty The un-fairness penalty of the resource
	 * @param params Parameters structure to fill
	 */
	void SetIndexParameters(uint64_t cfp, uint64_t cra,	float & penalty,
			CLEParams_t & params);

};

} // plugins

} // bbque

#endif // BBQUE_SC_FAIRNESS_
