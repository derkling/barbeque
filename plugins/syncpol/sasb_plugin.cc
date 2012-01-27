/**
 *       @file  sasb_plugin.cc
 *      @brief  The SASB synchronization policy
 *
 * This defines a dynamic C++ plugin which implements the "Starvation Avoidance
 * State Based" (SASB) heuristic for EXCc synchronizaiton.
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

#include "sasb_plugin.h"
#include "sasb_syncpol.h"
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

  // Registering SasbSincPolModule
  rp.CreateFunc = bp::SasbSyncPol::Create;
  rp.DestroyFunc = bp::SasbSyncPol::Destroy;
  res = params->RegisterObject(
		  (const char *)
		  SYNCHRONIZATION_POLICY_NAMESPACE SYNCHRONIZATION_POLICY_NAME, &rp);
  if (res < 0)
    return NULL;

  return PF_exitFunc;

}
PLUGIN_INIT(PF_initPlugin);

