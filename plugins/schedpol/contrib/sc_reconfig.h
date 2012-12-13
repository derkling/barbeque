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

#ifndef BBQUE_SC_RECONFIG_
#define BBQUE_SC_RECONFIG_

#include "sched_contrib.h"


// Proportional cost factor between MIGRATION and RECONFIGURATION
#define DEFAULT_MIGRATION_FACTOR 	4


namespace bbque { namespace plugins {


class SCReconfig: public SchedContrib {

public:

	/**
	 * @brief Constructor
	 */
	SCReconfig(const char * _name, uint16_t cfg_params[]);

	ExitCode_t Init(void * params);

private:

	/**
	 * Proportional factor meaning how many times the migration is more
	 * penalizing than a reconfiguration
	 */
	uint16_t migfact;

	/**
	 * @brief Compute the reconfiguration contribute
	 */
	ExitCode_t _Compute(SchedulerPolicyIF::EvalEntity_t const & evl_ent,
			float & ctrib);
};

} // plugins

} // bbque

#endif // BBQUE_SC_RECONFIG_


