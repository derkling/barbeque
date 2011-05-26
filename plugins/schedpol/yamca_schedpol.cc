/**
 *       @file  yamca_schedpol.cc
 *      @brief  The YaMCA resource scheduler (dynamic plugin)
 *
 * This implements a dynamic C++ plugin which implements the YaMCA resource
 * scheduler heuristic.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "yamca_schedpol.h"

#include <bbque/modules_factory.h>
#include <bbque/system_view.h>
#include <bbque/plugins/logger.h>

#include <iostream>

namespace bbque { namespace plugins {

YamcaSchedPol::YamcaSchedPol() {

	// Get a logger
	plugins::LoggerIF::Configuration conf(
			SCHEDULER_POLICY_NAMESPACE
			SCHEDULER_POLICY_NAME);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

	if (logger)
		logger->Info("YaMCA: Built a new dynamic object[%p]\n", this);
	else
		std::cout << "YaMCA: Build new dynamic object ["
			<< this << "]" << std::endl;

}

YamcaSchedPol::~YamcaSchedPol() {

}

//----- Scheduler policy module interface

char const * YamcaSchedPol::Name() {
	return SCHEDULER_POLICY_NAME;
}

SchedulerPolicyIF::ExitCode_t
YamcaSchedPol::Schedule(bbque::SystemView const & system) {
	//Silence "args not used" warning.
	(void)system;

	std::cout << "YaMCA: This is just a (working) dynamic module ["
		<< this << "]" << std::endl;
	// Put HERE your heuristic code

	return DONE;
}

//----- static plugin interface

void * YamcaSchedPol::Create(PF_ObjectParams *) {
	return new YamcaSchedPol();
}

int32_t YamcaSchedPol::Destroy(void * plugin) {
  if (!plugin)
    return -1;
  delete (YamcaSchedPol *)plugin;
  return 0;
}

} // namesapce plugins

} // namespace bque

