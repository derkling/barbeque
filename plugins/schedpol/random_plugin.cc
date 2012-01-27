/**
 *       @file  random_plugin.cc
 *      @brief  The Random resource scheduler (dynamic plugin)
 *
 * This implements a dynamic C++ plugin which implements the Random resource
 * scheduler heuristic.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
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

#include "random_plugin.h"
#include "random_schedpol.h"
#include "bbque/plugins/static_plugin.h"

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

  // Registering RandomSchedPolModule
  rp.CreateFunc = bp::RandomSchedPol::Create;
  rp.DestroyFunc = bp::RandomSchedPol::Destroy;
  res = params->RegisterObject(
		  (const char *)
		  SCHEDULER_POLICY_NAMESPACE SCHEDULER_POLICY_NAME, &rp);
  if (res < 0)
    return NULL;

  return PF_exitFunc;

}
PLUGIN_INIT(PF_initPlugin);

