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

#include <cmath>

#include "metrics_contribute.h"
#include "bbque/modules_factory.h"

#define MODULE_NAMESPACE SCHEDULER_POLICY_NAMESPACE".mct"

namespace bu = bbque::utils;
namespace po = boost::program_options;

namespace bbque { namespace plugins {


char const * MetricsContribute::ResourceGenPaths[MCT_RSRC_COUNT] = {
	RSRC_CLUST_PE,
	RSRC_CLUST_MEM
};

char const * MetricsContribute::ResourceNames[MCT_RSRC_COUNT] = {
	"pe",
	"mem"
};

char const * MetricsContribute::ConfigParamsStr[MCT_CPT_COUNT] = {
	"msl.pe",
	"msl.mem"
};

uint16_t const MetricsContribute::ConfigParamsDefault[MCT_CPT_COUNT] = {
	90,
	70,
};


MetricsContribute::MetricsContribute(const char * _name,
		uint16_t const params[]):
	cm(ConfigurationManager::GetInstance()) {

	// Identifier name of the contribute
	strncpy(name, _name, MCT_NAME_MAX_LEN);
	name[MCT_NAME_MAX_LEN-1] = '\0';

	// Array of Maximum Saturation Levels parameters
	for (int i = 0; i < MCT_RSRC_COUNT; ++i)
		msl_params[i] = static_cast<float> (params[i]) / 100.0;

	// Get a logger instance
	char logname[16];
	snprintf(logname, 16, MODULE_NAMESPACE".%s", name);
	plugins::LoggerIF::Configuration conf(logname);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
}

MetricsContribute::ExitCode_t
MetricsContribute::Compute(SchedulerPolicyIF::EvalEntity_t const & evl_ent,
		float & ctrib) {

	// A valid token for the resource state view is mandatory
	if (vtok == 0) {
		logger->Error("Missing a valid system/state view");
		return MCT_ERR_VIEW;
	}

	_Compute(evl_ent, ctrib);

	logger->Info("%s: %s = %.4f", evl_ent.StrId(), name, ctrib);
	assert((ctrib >= 0) && (ctrib <= 1));

	return MCT_SUCCESS;
}

void MetricsContribute::GetResourceThresholds(std::string const & rsrc_path,
		uint64_t rsrc_amount,
		SchedulerPolicyIF::EvalEntity_t const & evl_ent,
		ResourceThresholds_t & rl) {
	// Total amount of resource
	rl.total = sv->ResourceTotal(rsrc_path);

	// Get the max saturation level of the resource
	std::string rsrc_name(ResourcePathUtils::GetNameTemplate(rsrc_path));
	if (rsrc_name.compare(ResourceNames[0]) == 0)
		rl.saturate = rl.total * msl_params[0];
	else
		rl.saturate = rl.total * msl_params[1];

	// Resource availability (system resource state view)
	rl.free = sv->ResourceAvailable(rsrc_path, vtok);
	rl.usage = rl.total - rl.free;

	// Amount of resource remaining before reaching the saturation
	rl.sat_lack = 0;
	if (rl.free > rl.total - rl.saturate)
		rl.sat_lack = rl.saturate - rl.total + rl.free;

	assert(rl.sat_lack <= rl.free);
	logger->Debug("%s: Regions => usg: %lu| sat: %lu| sat-lack: %lu| "
			"free: %lu| req: %lu|",
			evl_ent.StrId(),
			rl.usage, rl.saturate, rl.sat_lack, rl.free, rsrc_amount);
}

float MetricsContribute::CLEIndex(uint64_t c_thresh,
		uint64_t l_thresh,
		float rsrc_amount,
		CLEParams_t const & params) {
	// SSR: Sub-Saturation Region
	if (rsrc_amount <= c_thresh) {
		logger->Debug("Region: ""Constant""");
		return params.k;
	}

	// ISR: In-Saturation Region
	if (rsrc_amount <= l_thresh) {
		logger->Debug("Region: ""Linear""");
		return FuncLinear(rsrc_amount, params.lin);
	}

	// OSR: Over-Saturation Region
	logger->Debug("Region: ""Exponential""");
	return FuncExponential(rsrc_amount, params.exp);
}

float MetricsContribute::FuncLinear(float x, LParams_t const & p) {
	DB(
		fprintf(stderr, FD("LIN ==== 1 - %.6f * (%.2f - %.2f)\n"),
			p.scale, x, p.xoffset);
	  );
	return 1 - p.scale * (x - p.xoffset);
}

float MetricsContribute::FuncExponential(float x, EParams_t const & p) {
	DB(
		fprintf(stderr, FD("EXP ==== %.4f * (%.4f ^ ((%.4f - %.4f) / %.4f) - 1)\n"),
			p.yscale, p.base, x, p.xoffset, p.xscale);
	  );
	return p.yscale * (pow(p.base, ((x - p.xoffset) / p.xscale)) - 1);
}


} // namespace plugins

} // namespace bbque
