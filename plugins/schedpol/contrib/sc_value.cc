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

#include "sc_value.h"

using namespace bbque::res;

namespace po = boost::program_options;

namespace bbque { namespace plugins {


SCValue::SCValue(const char * _name, uint16_t const cfg_params[]):
	SchedContrib(_name, cfg_params) {
}

SchedContrib::ExitCode_t
SCValue::Init(void * params) {
	(void) params;

	return SC_SUCCESS;
}

SchedContrib::ExitCode_t
SCValue::_Compute(SchedulerPolicyIF::EvalEntity_t const & evl_ent,
		float & ctrib) {
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
		return SC_SUCCESS;

	logger->Debug("%s: Normalized Actual Penalty (NAP) = %d/100): %.4f",
			evl_ent.StrId(), evl_ent.papp->GetGoalGap(), nap);

	// Add the NAP part to the contribute
	ctrib += nap;

	logger->Debug("%s: AWM Value index: %.4f", evl_ent.StrId(),	ctrib);
	return SC_SUCCESS;
}


} // namespace plugins

} // namespace bbque

