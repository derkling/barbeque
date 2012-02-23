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

#ifndef BBQUE_RESOURCE_ACCOUNTER_CONF_IF_H_
#define BBQUE_RESOURCE_ACCOUNTER_CONF_IF_H_

#include "bbque/resource_accounter_status.h"


namespace bbque {

/**
 * @brief Resources accounting configuration
 *
 * This virtual class provides the access to a subset of write interface of
 * the Resource Accounter.
 */
class ResourceAccounterConfIF: public ResourceAccounterStatusIF {

public:

	/**
	 * @brief Get a new resources view
	 *
	 * A component (core or module) can require a "personal" view of the
	 * resources. This means that ResourceAccounter "virtually clones"
	 * the system resources, blanking their states, allowing the component to
	 * make accounting without modify the real state of the resources.
	 *
	 * The component (i.e. Scheduler/Optimizer) should use the token returned
	 * with all the accounting methods, as a reference to the considered
	 * resources view. Note that a requiring component can manage more than
	 * one view.
	 *
	 * @param who_req A string identifying who requires the resource view
	 * @param tok The token to return for future references to the view
	 * @return RA_SUCCESS if a valid token has been returned.
	 * RA_ERR_MISS_PATH if the identifier path is empty.
	 */
	virtual ExitCode_t GetView(std::string who_req, RViewToken_t & tok) = 0;

	/**
	 * @brief Release a resources state view
	 *
	 * Remove the resources state view referenced by the token number.
	 *
	 * @param tok The token used as reference to the resources state view.
	 */
	virtual void PutView(RViewToken_t tok) = 0;

};

} // namespace bbque

#endif // BBQUE_RESOURCE_ACCOUNTER_CONF_IF_H_
