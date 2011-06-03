/**
 *       @file  aprox_plugin.cc
 *      @brief  ApplicationProxyTest plugin init and exit functions
 *
 * This defines a dynamic C++ plugin which which is used to test and evaluate
 * the ApplicationProxy component.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  05/06/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "aprox_test.h"
#include "aprox_plugin.h"

namespace bp = bbque::plugins;

extern "C"
int32_t PF_exitFunc() {
  return 0;
}

extern "C"
PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params) {
	int res = 0;

	PF_RegisterParams rp;
	rp.version.major = 1;
	rp.version.minor = 0;
	rp.programming_language = PF_LANG_CPP;

	// Registering ApplicationProxyTest Module
	rp.CreateFunc = bp::ApplicationProxyTest::Create;
	rp.DestroyFunc = bp::ApplicationProxyTest::Destroy;
	res = params->RegisterObject((const char *)TEST_NAMESPACE"aprox" , &rp);
	if (res < 0)
		return NULL;

	return PF_exitFunc;

}
PLUGIN_INIT(PF_initPlugin);
