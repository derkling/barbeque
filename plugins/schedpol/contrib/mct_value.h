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

#ifndef BBQUE_MCT_VALUE_
#define BBQUE_MCT_VALUE_

#include "metrics_contribute.h"


namespace bbque { namespace plugins {


class MCTValue: public MetricsContribute {

public:

	/**
	 * @brief Constructor
	 *
	 * @param name A name identifying the specific contribute
	 * @param cfg_params Global configuration parameters
	 */
	MCTValue(const char * _name, uint16_t const cfg_params[]);

	ExitCode_t Init(void * params);

private:

	/**
	 * @brief Compute the AWM value contribute
	 *
	 * The contribute starts from the static value associated to the AWM to
	 * evaluate. If a "goal gap" has been set, a "target" resource usage is
	 * considered accordingly. If the AWM to evaluate provides a resource
	 * usage greater or equal to the target usage, the static AWM value is
	 * returned as contribute index, otherwise the index will be a
	 * "penalization" of the static value.
	 *
	 * @param evl_ent The scheduling entity to evaluate
	 * @ctrib ctrib The contribute index to return
	 *
	 * @return MCT_SUCCESS. No error conditions expected.
	 */
	ExitCode_t _Compute(EvalEntity_t const & evl_ent, float & ctrib);

};

} // plugins

} // bbque

#endif // BBQUE_MCT_VALUE_


