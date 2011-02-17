/**
 *       @file  plugin.cc
 *      @brief  A static Logger plugin based on Log4CPP
 *
 * This defines a static Log4CPP based plugin which instantiate an object
 * implementing the LoggerIF interface.
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
 * =====================================================================================
 */

#include "log4cpp_plugin.h"

#include "log4cpp_logger.h"
#include "bbque/platform_services.h"
#include "bbque/plugins/static_plugin.h"


namespace bp = bbque::plugins;

extern "C"
int32_t StaticPlugin_Log4CppLogger_ExitFunc() {
	return 0;
}

extern "C"
PF_ExitFunc StaticPlugin_Log4CppLogger_InitPlugin(const PF_PlatformServices * params) {
	int res = 0;

	// Setting-up plugins registration info
	PF_RegisterParams rp;
	rp.version.major = 1;
	rp.version.minor = 0;
	rp.programming_language = PF_LANG_CPP;

	// Registering Log4CPP
	rp.CreateFunc = bp::Log4CppLogger::Create;
	rp.DestroyFunc = bp::Log4CppLogger::Destroy;
	res = params->RegisterObject((const char *)LOGGER_NAMESPACE"log4cpp", &rp);
	if (res < 0)
		return NULL;

	return StaticPlugin_Log4CppLogger_ExitFunc;

}

bp::StaticPlugin
StaticPlugin_Log4CppLogger(StaticPlugin_Log4CppLogger_InitPlugin);

