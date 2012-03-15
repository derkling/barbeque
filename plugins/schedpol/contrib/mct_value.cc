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

#include "mct_value.h"

using namespace bbque::res;

namespace po = boost::program_options;

namespace bbque { namespace plugins {

/** Set default congestion penalties values */
uint16_t MCTValue::penalties_default[MCT_RSRC_COUNT] = {
	10,
	10
};

MCTValue::MCTValue(const char * _name, uint16_t const cfg_params[]):
	MetricsContribute(_name, cfg_params) {
	char conf_str[50];

	// Configuration parameters
	po::options_description opts_desc("AWM value contribute parameters");

	// Congestion penalties
	for (int i = 0; i < MCT_RSRC_COUNT; ++i) {
		snprintf(conf_str, 50, MCT_CONF_BASE_STR"%s.penalty.%s",
				name, ResourceNames[i]);

		logger->Debug("%s", conf_str);
		opts_desc.add_options()
			(conf_str,
			 po::value<uint16_t>
				(&penalties_int[i])->default_value(penalties_default[i]),
			 "AWM value penalty per resource");
		;
	}

	po::variables_map opts_vm;
	cm.ParseConfigurationFile(opts_desc, opts_vm);

	// Boundaries enforcement (0 <= penalty <= 100)
	for (int i = 0; i < MCT_RSRC_COUNT; ++i) {
		if (penalties_int[i] > 100) {
			logger->Warn("Parameter penalty.%s out of range [0,100]: "
					"found %d. Setting to %d", ResourceNames[i],
					penalties_int[i], penalties_default[i]);
			penalties_int[i] = penalties_default[i];
		}
		penalties[i] = static_cast<float>(penalties_int[i]) / 100.0;
		logger->Debug("penalty.%s \t= %.2f", ResourceNames[i], penalties[i]);
	}
}

MetricsContribute::ExitCode_t
MCTValue::Init(void * params) {
	(void) params;

	return MCT_SUCCESS;
}

MetricsContribute::ExitCode_t
MCTValue::_Compute(EvalEntity_t const & evl_ent, float & ctrib) {
	std::string rsrc_tmp_path;
	UsagesMap_t::const_iterator usage_it;
	AwmPtr_t const & curr_awm(evl_ent.papp->CurrentAWM());
	float nap = 0;

	// Initialize the index contribute to the AWM static value
	ctrib = 0.4 * evl_ent.pawm->Value();
	logger->Debug("%s: AWM static value: %.4f", evl_ent.StrId(), ctrib);

	// NAP set?
	nap = 0.6 * static_cast<float>(evl_ent.papp->GetGoalGap()) / 100.0;
	if (!curr_awm || (nap == 0) ||
			(curr_awm->Value() >= evl_ent.pawm->Value()))
		return MCT_SUCCESS;

	logger->Debug("%s: Normalized Actual Penalty (NAP) = %d/100): %.4f",
			evl_ent.StrId(), evl_ent.papp->GetGoalGap(), nap);

	// Add the NAP part to the contribute
	ctrib += nap;

	logger->Debug("%s: AWM Value index: %.4f", evl_ent.StrId(),	ctrib);
	return MCT_SUCCESS;
}


} // namespace plugins

} // namespace bbque

