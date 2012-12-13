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


#include <cstring>
#include <numeric>

#include "bbque/modules_factory.h"
#include "sched_contrib_manager.h"

#include "mct_value.h"
#include "mct_reconfig.h"
#include "mct_congestion.h"
#include "mct_fairness.h"
// ...:: ADD_SC ::...

namespace po = boost::program_options;

namespace bbque { namespace plugins {

/*****************************************************************************
 *                         Static data initialization                        *
 *****************************************************************************/


bool SchedContribManager::config_ready = false;

char const * SchedContribManager::sc_str[SC_COUNT] = {
	"awmvalue",
	"reconfig",
	"congestion",
	"fairness"
	//"power",
	//"thermal",
	//"stability",
	//"robustness"
	// ...:: ADD_SC ::...
};

std::map<const char *, SchedContribPtr_t> SchedContribManager::sc_objs = {};
float SchedContribManager::sc_weights_norm[SC_COUNT] = {0};
uint16_t SchedContribManager::sc_weights[SC_COUNT] = {0};
uint16_t SchedContribManager::sc_cfg_params[SchedContrib::MCT_CPT_COUNT] = {0};


/*****************************************************************************
 *                       Public member functions                             *
 *****************************************************************************/

SchedContribManager::SchedContribManager(
		SCType_t const * sc_types,
		uint8_t sc_num):
	cm(ConfigurationManager::GetInstance()) {

	// Get a logger
	plugins::LoggerIF::Configuration conf(MODULE_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (logger)
		logger->Info("Built a new dynamic object[%p]", this);
	else
		fprintf(stderr, FI("%s: Built new dynamic object [%p]\n"),
				SC_MANAGER_NAMESPACE, (void *)this);

	// Parse the configuration parameters
	if (!SchedContribManager::config_ready) {
		ParseConfiguration();
		NormalizeWeights();
		AllocateContribs();
		SchedContribManager::config_ready = true;
	}

	// Init the map of scheduling contributions required
	for (int i = 0; i < sc_num; ++i) {

		switch (sc_types[i]) {

		case VALUE:
			sc_objs_reqs[sc_str[VALUE]] = SchedContribManager::sc_objs[sc_str[VALUE]];
			break;
		case RECONFIG:
			sc_objs_reqs[sc_str[RECONFIG]] = SchedContribManager::sc_objs[sc_str[RECONFIG]];
			break;
		case CONGESTION:
			sc_objs_reqs[sc_str[CONGESTION]] = SchedContribManager::sc_objs[sc_str[CONGESTION]];
			break;
		case FAIRNESS:
			sc_objs_reqs[sc_str[FAIRNESS]] = SchedContribManager::sc_objs[sc_str[FAIRNESS]];
			break;
		default:
			logger->Error("Scheduling contribution unknown: %d", sc_types[i]);
		}
	}
}


SchedContribManager::~SchedContribManager() {
	sc_objs.clear();
}

SchedContribManager::ExitCode_t SchedContribManager::GetIndex(
		SCType_t sc_type,
		SchedulerPolicyIF::EvalEntity_t const & evl_ent,
		float &  sc_value,
		SchedContrib::ExitCode_t & sc_ret,
		bool weighed) {
	std::map<const char *, SchedContribPtr_t>::iterator sc_it;
	
	// Boundary check
	if (sc_type >= SC_COUNT)
		return SC_TYPE_UNKNOWN;

	// Get the SchedContrib object
	sc_it = sc_objs_reqs.find(sc_str[sc_type]);
	if (sc_it == sc_objs_reqs.end())
		return SC_TYPE_MISSING;

	// Compute the SchedContrib index
	sc_ret = (*sc_it).second->Compute(evl_ent, sc_value);
	if (unlikely(sc_ret != SchedContrib::MCT_SUCCESS))
		return SC_ERROR;

	// Multiply the index for the weight
	if (weighed)
		sc_value *= sc_weights_norm[sc_type];

	return OK;
}

SchedContribPtr_t SchedContribManager::GetContrib(SCType_t sc_type) {
	std::map<const char *, SchedContribPtr_t>::iterator sc_it;

	// Find the SchedContrib object
	sc_it = sc_objs_reqs.find(sc_str[sc_type]);
	if (sc_it == sc_objs_reqs.end())
		return SchedContribPtr_t();

	return (*sc_it).second;
}

void SchedContribManager::SetViewInfo(System * sv, RViewToken_t vtok) {
	std::map<const char *, SchedContribPtr_t>::iterator sc_it;

	// For each SchedContrib set the resource view information 
	for (sc_it = sc_objs_reqs.begin(); sc_it != sc_objs_reqs.end(); ++sc_it)
		(*sc_it).second->SetViewInfo(sv, vtok);
}


/*****************************************************************************
 *                       Private member functions                            *
 *****************************************************************************/


void SchedContribManager::ParseConfiguration() {
	char conf_opts[SC_COUNT+SchedContrib::MCT_CPT_COUNT][40];

	// Load the weights of the metrics contributes
	po::options_description opts_desc("Scheduling contributions parameters");
	for (int i = 0; i < SC_COUNT; ++i) {
		snprintf(conf_opts[i], 40, MODULE_CONFIG".%s.weight", sc_str[i]);
		opts_desc.add_options()
			(conf_opts[i],
			 po::value<uint16_t> (&sc_weights[i])->default_value(0),
			"Single contribution weight");
		;
	}

	// Global SchedContrib config parameters
	for (int i = 0; i < SchedContrib::MCT_CPT_COUNT; ++i) {
		snprintf(conf_opts[SC_COUNT+i], 40, MCT_CONF_BASE_STR".%s",
				SchedContrib::ConfigParamsStr[i]);
		opts_desc.add_options()
			(conf_opts[SC_COUNT+i],
			 po::value<uint16_t>
			 (&sc_cfg_params[i])->default_value(
				 SchedContrib::ConfigParamsDefault[i]),
			"MCT global parameters");
		;
	}
	po::variables_map opts_vm;
	cm.ParseConfigurationFile(opts_desc, opts_vm);

	// Boundaries enforcement (0 <= MSL <= 100)
	for (int i = 0; i < SchedContrib::MCT_CPT_COUNT; ++i) {
		logger->Debug("Resource [%s] min saturation level \t= %d [%]",
				(strpbrk(SchedContrib::ConfigParamsStr[i], "."))+1,
				sc_cfg_params[i]);
		if (sc_cfg_params[i] > 100) {
			logger->Warn("Parameter %s out of range [0,100]: found %d. Setting to %d",
					SchedContrib::ConfigParamsStr[i],
					sc_cfg_params[i],
					SchedContrib::ConfigParamsDefault[i]);
			sc_cfg_params[i] = SchedContrib::ConfigParamsDefault[i];
		}
	}
}

void SchedContribManager::NormalizeWeights() {
	uint16_t sum = 0;

	// Accumulate (sum) the weights
	sum = std::accumulate(sc_weights, sc_weights + SC_COUNT, 0);

	// Normalize
	for (int i = 0; i < SC_COUNT; ++i) {
		sc_weights_norm[i] = sc_weights[i] / (float) sum;
		logger->Debug("Contribution [%.*s] weight \t= %.3f", 5,
				sc_str[i], sc_weights_norm[i]);
	}
}

void SchedContribManager::AllocateContribs() {

	// Init the map of scheduling contribution objects
	sc_objs[sc_str[VALUE]] = SchedContribPtr_t(
			new MCTValue(sc_str[VALUE],	sc_cfg_params));
	sc_objs[sc_str[RECONFIG]] = SchedContribPtr_t(
			new MCTReconfig(sc_str[RECONFIG], sc_cfg_params));
	sc_objs[sc_str[CONGESTION]] = SchedContribPtr_t(
			new MCTCongestion(sc_str[CONGESTION], sc_cfg_params));
	sc_objs[sc_str[FAIRNESS]] = SchedContribPtr_t(
			new MCTFairness(sc_str[FAIRNESS], sc_cfg_params));
	// ...:: ADD_SC ::...
}

} // namespace plugins

} // namespace bbque
