/**
 *       @file  resource_manager.h
 *      @brief  The Barbeque Run-Time Resource Manager
 *
 * This class provides the implementation of the Run-Time Resource Manager
 * (RTRM), which is the main barbeque module implementing its glue code.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  02/01/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_RESOURCE_MANAGER_H_
#define BBQUE_RESOURCE_MANAGER_H_

#include "bbque/platform_services.h"
#include "bbque/plugin_manager.h"

namespace bbque {

	/**
	 * @class ResourceManager
	 * @brief The class implementing the glue logic of Barbeque
	 * This class provides the glue logic of the Barbeque RTRM. Its 'Go'
	 * method represents the entry point of the run-time manager and it is
	 * called by main right after some playground preparation activities.
	 * This class is in charge to load all needed modules and run the control
	 * loop.
	 */
class ResourceManager {

public:

	/**
	 * @brief Get a reference to the resource manager
	 * The ResourceManager is a singleton class providing the glue logic for
	 * the Barbeque run-time resource manager. An instance to this class is to
	 * be obtained by main in order to start grilling
	 * @return  a reference to the ResourceManager singleton instance
	 */
	static ResourceManager & GetInstance();

	/**
	 * @brief Start managing resources
	 * This is the actual run-time manager entry-point, after the playground
	 * setup the main should call this method to start grilling applications.
	 * This will be in charge to load all the required modules and then start
	 * the control cycle.
	 */
	void Go();

private:

	/**
	 * @brief   Build a new instance of the resource manager
	 */
	ResourceManager();

	/**
	 * @brief  Clean-up the grill by releasing current resource manager
	 * resources and modules.
	 */
	~ResourceManager();

	/**
	 * @brief   The run-time resource manager control loop
	 * This provides the Barbeuqe applications and resources control logic.
	 * This is actually the entry point of the Barbeque state machine.
	 */
	void ControlLoop();

private:

	/**
	 * Set true when Barbeuque should terminate
	 */
	bool done;

	/**
	 * Reference to supported platform services class.
	 * The platform services are a set of services exported by Barbeque to
	 * other modules (and plugins). The resource manager ensure an
	 * initialization of this core module before starting to grill.
	 */
	PlatformServices & ps;

	/**
	 * Reference to the plugin manager module.
	 * The plugin manager is the required interface to load other modules. The
	 * resource manager ensure an initialization of this core module before
	 * starting to grill.
	 */
	plugins::PluginManager & pm;

};

} // namespace bbque

#endif // BBQUE_RESOURCE_MANAGER_H_

