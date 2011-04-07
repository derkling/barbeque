/**
 *       @file  static_test_plugin.cc
 *      @brief  A static plugin example
 *
 * This defines an example of static C++ plugin which instantiate an object
 * implementing the TestingObjectIF interface.
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

#include "dummy_plugin.h"
#include "dummy_test.h"
#include "bbque/plugins/static_plugin.h"

namespace bp = bbque::plugins;

#ifndef BBQUE_DYNAMIC_PLUGIN
# define	PLUGIN_NAME "dummy"
#else
# define	PLUGIN_NAME "dummy_dyn"
#endif

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

  // Registering DummyModule
  rp.CreateFunc = bp::DummyTest::Create;
  rp.DestroyFunc = bp::DummyTest::Destroy;
  res = params->RegisterObject((const char *)TEST_NAMESPACE PLUGIN_NAME, &rp);
  if (res < 0)
    return NULL;

  return PF_exitFunc;

}

#ifdef BBQUE_DYNAMIC_PLUGIN
PLUGIN_INIT(PF_initPlugin);
#else
bp::StaticPlugin
StaticPlugin_DummyTest(PF_initPlugin);
#endif

