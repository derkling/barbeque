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


#ifndef BBQUE_SCHED_CONTRIB_MANAGER_H_
#define BBQUE_SCHED_CONTRIB_MANAGER_H_

#include <map>

#include "sched_contrib.h"

#include "bbque/system.h"
#include "bbque/configuration_manager.h"

#define SC_MANAGER_NAMESPACE "scm"
#define SC_MANAGER_CONFIG    "Contrib"
#define MODULE_NAMESPACE SCHEDULER_POLICY_NAMESPACE"."SC_MANAGER_NAMESPACE
#define MODULE_CONFIG    SCHEDULER_POLICY_CONFIG"."SC_MANAGER_CONFIG

#define YAMS_SC_COUNT 	4

namespace bbque { namespace plugins {


/** Shared pointer to a metrics contribute */
typedef std::shared_ptr<SchedContrib> SchedContribPtr_t;

/**
 * @brief Manager of Scheduling Contributions (once "Metrics")
 */
class SchedContribManager {

public:

	enum ExitCode_t {
		OK = 0,
		SC_TYPE_UNKNOWN,
		SC_TYPE_MISSING,
		SC_ERROR
	};

	/**
	 * @brief Types of scheduling metrics contributions
	 */
	enum SCType_t {
		VALUE = 0,
		RECONFIG,
		CONGESTION,
		FAIRNESS,
		//POWER,
		//THERMAL,
		//STABILITY,
		//ROBUSTNESS,
		// ...:: ADD_SC ::...

		SC_COUNT
	};

	/**
	 * @brief Scheduling Contributions Manager constructor
	 *
	 * @param sc_types Array of contributions type required
	 * @param sc_num Number of contributions required
	 */
	SchedContribManager(SCType_t const * sc_types, uint8_t sc_num);

	/**
	 * @brief Scheduling Contributions Manager destructor
	 */
	virtual ~SchedContribManager();

	/**
	 * @brief Compute a specific scheduling contribution index
	 *
	 * @param sc_types Array of contributions type required
	 * @param evl_ent The scheduling entity to evaluate
	 * @param sc_value The SchedContrib index value
	 * @param sc_ret The return code of the SchedContrib computation
	 * @param weighed if true (default) the function multiplies the index for
	 * the weight
	 *
	 * @return 
	 */
	SchedContribManager::ExitCode_t GetIndex(SCType_t sc_type,
			SchedulerPolicyIF::EvalEntity_t const & evl_ent,
			float & sc_value, SchedContrib::ExitCode_t & sc_ret,
			bool weighed = true);

	/**
	 * @brief Get a specific scheduling contribution object
	 *
	 * @param sc_type The scheduling contribution type
	 *
	 * @return A shared pointer to a SchedContrib object
	 */
	SchedContribPtr_t GetContrib(SCType_t sc_type);

	/**
	 * @brief Get the string of the SchedContrib type name
	 *
	 * @param sc_type The scheduling contribution type
	 *
	 * @return the SchedContrib type as a char string
	 */
	inline const char * GetString(SCType_t sc_type) {
		return sc_str[sc_type];
	}

	/**
	 * @brief Get the total number of SchedContrib registered
	 *
	 * @return How many SChecontrib have been registered
	 */
	inline uint8_t GetNumMax() const {
		return SC_COUNT;
	}

	/**
	 *@brief Set scheduling base information for each SchedContrib
	 *
	 * This sets the resource state view of the current scheduling run, and
	 * reference to the System interface
	 */
	void SetViewInfo(System * sv, RViewToken_t vtok);

private:

	/** System logger instance */
	LoggerIF *logger;

	/** Configuration manager instance */
	ConfigurationManager & cm;

	/** Track if a SCM has been previously instanciated */
	static bool config_ready;


	/** Scheduling contributions required*/
	std::map<const char *, SchedContribPtr_t> sc_objs_reqs;

	/** Scheduling contributions (all) */
	static std::map<const char *, SchedContribPtr_t> sc_objs;


	/** Metrics contribute configuration keys */
	static char const * sc_str[SC_COUNT];

	/** Normalized metrics contributes weights */
	static float sc_weights_norm[SC_COUNT];
	
	/** Metrics contributes weights */
	static uint16_t sc_weights[SC_COUNT];

	/** Global config parameters for metrics contributes */
	static uint16_t sc_cfg_params[SchedContrib::SC_CPT_COUNT];


	/**
	 * @brief Parse all the SchedContrib configuration parameters
	 */
	void ParseConfiguration();

	/**
	 * @brief Normalize the weights paramenters
	 */
	void NormalizeWeights();

	/**
	 * @brief Allocate all the SchedContrib ojects
	 */
	void AllocateContribs();

};

}	// namespace plugins

}	// namespace bbque

#endif // BBQUE_SCHED_CONTRIB_MANAGER_H_
