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

namespace bbque { namespace plugins {

MCTValue::MCTValue(const char * _name, uint16_t const cfg_params[]):
	MetricsContribute(_name, cfg_params) {
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
	uint8_t ggap = 0;
	uint64_t curr_awm_rsrc_usage;
	float target_usage = 0.0;
	float index;

	// Pre-set the index contribute to the AWM static value
	ctrib = evl_ent.pawm->Value();

	// Goal-Gap set?
	ggap = evl_ent.papp->GetGoalGap();
	logger->Debug("%s: Goal Gap: %d", evl_ent.StrId(), ggap);
	if (!curr_awm || (ggap == 0))
		return MCT_SUCCESS;

	// Resource usages of the current entity (AWM + Cluster)
	for_each_sched_resource_usage(evl_ent, usage_it) {
		std::string const & rsrc_path(usage_it->first);
		UsagePtr_t const & pusage(usage_it->second);

		// Resource usage of the current AWM
		rsrc_tmp_path =	ResourcePathUtils::GetTemplate(rsrc_path);
		curr_awm_rsrc_usage = curr_awm->ResourceUsageAmount(rsrc_tmp_path);
		logger->Debug("%s: R{%s} usage: in curr AWM = %lu| in cand AWM = %lu",
				evl_ent.StrId(), rsrc_tmp_path.c_str(),
				curr_awm_rsrc_usage, pusage->GetAmount());

		// Target resource usage due to the goal gap assertion
		target_usage = static_cast<float>(curr_awm_rsrc_usage) *
			(1.0 + static_cast<float>(ggap) / 100.0);
		logger->Debug("%s: R{%s} target usage: %.2f", evl_ent.StrId(),
				rsrc_tmp_path.c_str(), target_usage);

		// Skip the resource whether it satisfies the goal-gap usage requirement
		if (pusage->GetAmount() >= target_usage)
			continue;

		// Compute the contribute index (with penalization)
		index = evl_ent.pawm->Value() * pusage->GetAmount() / target_usage;
		logger->Debug("%s: R{%s} index: %.4f", evl_ent.StrId(),
				rsrc_tmp_path.c_str(), index);

		// Get the index related to the most penalizing resource usage
		if (index < ctrib)
			ctrib = index;
	}

	return MCT_SUCCESS;
}


} // namespace plugins

} // namespace bbque

