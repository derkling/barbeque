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
	res = params->RegisterObject((const char *)MODULE_NAMESPACE, &rp);
	if (res < 0)
		return NULL;

	return StaticPlugin_Log4CppLogger_ExitFunc;

}

#ifdef BBQUE_DYNAMIC_PLUGIN
PLUGIN_INIT(PF_initPlugin);
#else
bp::StaticPlugin
StaticPlugin_Log4CppLogger(StaticPlugin_Log4CppLogger_InitPlugin);
#endif

