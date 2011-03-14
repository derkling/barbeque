/**
 *       @file  modules_factory.h
 *      @brief  A modules factory class
 *
 * This provides a factory of barbeuque modules. Each module could be build by
 * the core framework thanks to a correponfing method of this singleton
 * Factory class.
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

#ifndef BBQUE_MODULES_FACTORY_H_
#define BBQUE_MODULES_FACTORY_H_

//----- Supported plugin interfaces
#include "bbque/plugins/test_adapter.h"
#include "bbque/plugins/logger_adapter.h"
#include "bbque/plugins/recipe_loader.h"
//----- Supported C++ only plugin interfaces
#include "bbque/plugins/scheduler_policy.h"

#include <string>

namespace bbque {

/**
 * @class ModulesFactory
 * @brief A singleton class to build other Barbeque modules.
 * This class provides a set of factory methods to build other Barbeque
 * modules. By design, within Barbeque each component (except core components
 * such this one)  is represented by one or more module which could be either
 * statically linked or dynamically loaded. Each such module should implement
 * a predefined interface, thus making them interchangable.
 * This class provides a set of factory methods to get a reference to each
 * kind of supported interfaces.
 */
class ModulesFactory {

public:

	/**
	 * Get a reference to the (singleton) module factory
	 */
	static ModulesFactory & GetInstance();

	/**
	 * Get a reference to a module implementing the TestIF interface
	 */
	static plugins::TestIF * GetTestModule(
			const std::string & id = TEST_NAMESPACE);

	/**
	 * Get a reference to a module implementing the LoggerIF interface
	 */
	static plugins::LoggerIF * GetLoggerModule(
			plugins::LoggerIF::Configuration const & data,
			std::string const & id = LOGGER_NAMESPACE);

	/**
	 * Get a reference to a module implementing the RecipeLoaderIF interface
	 */
	static plugins::RecipeLoaderIF * GetRecipeLoaderModule(
			std::string const & id = RECIPE_LOADER_NAMESPACE);

	/**
	 * Get a reference to a module implementing the SchedulerPolicyIF interface
	 */
	static plugins::SchedulerPolicyIF * GetSchedulerPolicyModule(
		std::string const & id = SCHEDULER_POLICY_NAMESPACE);

private:

	/**
	 * @brief   Build a new modules factory
	 */
	ModulesFactory();

};

} // namespace bbque

#endif // BBQUE_MODULES_FACTORY_H_

