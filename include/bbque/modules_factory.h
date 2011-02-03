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

#include <string>

namespace bbque {

class ModulesFactory {

public:

	static ModulesFactory & GetInstance();

	static plugins::TestIF * GetTestModule(const std::string & objectType);

	static plugins::LoggerIF * GetLoggerModule(const std::string & objectType);

private:

	/**
	 * @brief   Build a new modules factory
	 */
	ModulesFactory();

};

} // namespace bbque

#endif // BBQUE_MODULES_FACTORY_H_

