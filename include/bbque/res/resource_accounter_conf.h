/**
 *       @file  resource_accounter_conf.h
 *      @brief  "Update" ResourceAccounter status interface
 *
 * This defines the "update" status interface for ResourceAccounter.
 * This interface allows the update of resource states information. For
 * instance, when the Application Manager receive a notify upon an Application
 * working mode switch. In such case an update about the resource usages is
 * needed.
 * Moreover when Barbeque check for the platform resources, we need a method
 * for register the resources, filling a descriptor with the information
 * exposed by the platform.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  04/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_RESOURCE_ACCOUNTER_CONF_IF_H_
#define BBQUE_RESOURCE_ACCOUNTER_CONF_IF_H_

#include "bbque/res/resource_accounter_status.h"

using bbque::app::AppPtr_t;
using bbque::app::Application;

namespace bbque { namespace res {

/**
 * @class ResourceAccounterConfIF
 *
 * This interface allow the update of runtime resource information into the
 * Resource Accounter component.
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
	 * @return @see ExitCode_t
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

} // namespace res

} // namespace bbque

#endif // BBQUE_RESOURCE_ACCOUNTER_CONF_IF_H_

