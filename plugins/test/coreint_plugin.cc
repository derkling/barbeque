/**
 *       @file  coreint_plugin.cc
 *      @brief  CoreInteractionsTest plugin glue-code
 *
 * This implement functions for loading (and removing) CoreInteractionsTest
 * plugin.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  06/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "coreint_test.h"
#include "coreint_plugin.h"

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

	// Registering CoreInteractionsTest Module
	rp.CreateFunc = bp::CoreInteractionsTest::Create;
	rp.DestroyFunc = bp::CoreInteractionsTest::Destroy;
	res = params->RegisterObject((const char *)TEST_NAMESPACE"coreint" , &rp);
	if (res < 0)
		return NULL;

	return PF_exitFunc;

}
PLUGIN_INIT(PF_initPlugin);

