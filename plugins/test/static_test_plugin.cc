/**
 *       @file  static_test_plugin.cc
 *      @brief  A static plugin example
 *
 * This defines an example of static C++ plugin which instantiate an object
 * implementing the TestingObjectIF interface.
 *
 * Detailed description starts here.
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

#include "static_test_plugin.h"
#include "static_test_object.h"
#include "bbque/plugins/static_plugin.h"

namespace bp = bbque::plugins;

extern "C"
int32_t StaticPlugin_DummyModule_ExitFunc() {
  return 0;
}

extern "C"
PF_ExitFunc StaticPlugin_DummyModule_InitPlugin(const PF_PlatformServices * params) {
  int res = 0;

  PF_RegisterParams rp;
  rp.version.major = 1;
  rp.version.minor = 0;
  rp.programming_language = PF_LANG_CPP;

  // Registering DummyModule
  rp.CreateFunc = bp::DummyModule::Create;
  rp.DestroyFunc = bp::DummyModule::Destroy;
  res = params->RegisterObject((const char *)"DummyModule", &rp);
  if (res < 0)
    return NULL;

  return StaticPlugin_DummyModule_ExitFunc;

}

bp::StaticPlugin
StaticPlugin_DummyModule(StaticPlugin_DummyModule_InitPlugin);

