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

#include "mct_congestion.h"

namespace po = boost::program_options;

namespace bbque { namespace plugins {

/** Set default congestion penalties values */
uint16_t MCTCongestion::penalties_default[MCT_RSRC_COUNT] = {
	75,
	50
};

MCTCongestion::MCTCongestion(const char * _name, uint16_t const cfg_params[]):
	MetricsContribute(_name, cfg_params) {
	char conf_str[50];

	// Configuration parameters
	po::options_description opts_desc("Congestion contribute parameters");

	// Base for exponential
	snprintf(conf_str, 50, MCT_CONF_BASE_STR"%s.expbase", name);
	opts_desc.add_options()
		(conf_str,
		 po::value<uint16_t>(&expbase)->default_value(DEFAULT_CONG_EXPBASE),
		 "Base for exponential function");
		;

	// Congestion penalties
	for (int i = 0; i < MCT_RSRC_COUNT; ++i) {
		snprintf(conf_str, 50, MCT_CONF_BASE_STR"%s.penalty.%s",
				name, rsrc_types_str[i]);

		logger->Debug("%s", conf_str);
		opts_desc.add_options()
			(conf_str,
			 po::value<uint16_t>
			 	(&penalties_int[i])->default_value(penalties_default[i]),
			 "Congestion penalty per resource");
		;
	}

	po::variables_map opts_vm;
	cm.ParseConfigurationFile(opts_desc, opts_vm);

	// Boundaries enforcement (0 <= penalty <= 100)
	for (int i = 0; i < MCT_RSRC_COUNT; ++i) {
		if (penalties_int[i] > 100) {
			logger->Warn("Parameter penalty.%s out of range [0,100]: "
					"found %d. Setting to %d", rsrc_types_str[i],
					penalties_int[i], penalties_default[i]);
			penalties_int[i] = penalties_default[i];
		}
		penalties[i] = (float) penalties_int[i] / 100;
		logger->Debug("penalty.%s \t= %.2f", rsrc_types_str[i], penalties[i]);
	}
}

MetricsContribute::ExitCode_t
MCTCongestion::_Compute(EvalEntity_t const & evl_ent, float & ctrib) {
	UsagesMap_t::const_iterator usage_it;
	Region_t region;
	RegionLevels_t rl;
	CLEParams_t params;
	float ru_index;

	ctrib = 1.0;

	// Fixed function parameters
	params.k = 1.0;
	params.lin.yscale = -1.0;
	params.lin.yoffset = 1.0;
	params.exp.base = expbase;

	// Iterate the whole set of resource usage
	for_each_sched_resource_usage(evl_ent, usage_it) {
		std::string const & rsrc_path(usage_it->first);
		UsagePtr_t const & pusage(usage_it->second);
		logger->Debug("%s: {%s}", evl_ent.StrId(), rsrc_path.c_str());

		// Get the region of the (next) resource usage
		region = GetUsageRegion(rsrc_path, pusage->GetAmount(), evl_ent, rl);

		// 1. Get the congestion penalty to use
		// 2. Finish to set the parameters for the index computation
		std::string rsrc_name(ResourcePathUtils::GetNameTemplate(rsrc_path));
		if (rsrc_name.compare(rsrc_types_str[MCT_RSRC_PE]) == 0)
			SetIndexParameters(rl, penalties[MCT_RSRC_PE], params);
		else
			SetIndexParameters(rl, penalties[MCT_RSRC_MEM], params);

		// Compute the region index
		ru_index = ComputeCLEIndex(region, pusage->GetAmount(), params);
		logger->Debug("%s: {%s} index = %.4f", evl_ent.StrId(),
				rsrc_path.c_str(), ru_index);

		// Update the contribute if the index is lower, i.e. the most
		// penalizing request dominates
		ru_index < ctrib ? ctrib = ru_index: ctrib;
	}

	return MCT_SUCCESS;
}

void MCTCongestion::SetIndexParameters(RegionLevels_t const & rl, float & penalty,
		CLEParams_t & params) {

	// Linear parameters
	if (rl.free > 0) {
		params.lin.xoffset = -rl.usage;
		params.lin.xscale = penalty / rl.free;
	}
	params.lin.xoffset = static_cast<float>(rl.sat_lack);

	// Exponential parameters
	params.exp.yscale = (1.0 - penalty) / (params.exp.base - 1.0);
	params.exp.xscale = 1.0 / (rl.saturate - rl.total);
	params.exp.xoffset = -rl.total;
	params.exp.yoffset = -params.exp.yscale;
}


} // namespace plugins

} // namespace bbque
