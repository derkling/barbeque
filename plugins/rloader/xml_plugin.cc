/**
 *       @file  xml_plugin.cc
 *      @brief  Plugin management functions for XMLRecipeLoader
 *      (implementation)
 *
 * This implement the init and exit functions for the plugin XMLRecipeLoader.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  01/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "xml_plugin.h"
#include "xml_rloader.h"

#include "bbque/plugins/static_plugin.h"

namespace bp = bbque::plugins;

extern "C"
int32_t StaticPlugin_XMLRecipeLoader_ExitFunc() {
  return 0;
}

extern "C"
PF_ExitFunc StaticPlugin_XMLRecipeLoader_InitPlugin(const PF_PlatformServices * params) {
  int res = 0;

  PF_RegisterParams rp;
  rp.version.major = 1;
  rp.version.minor = 0;
  rp.programming_language = PF_LANG_CPP;

  // Registering XMLRecipeLoader
  rp.CreateFunc = bp::XMLRecipeLoader::Create;
  rp.DestroyFunc = bp::XMLRecipeLoader::Destroy;
  res = params->RegisterObject((const char *)RECIPE_LOADER_NAMESPACE "xml", &rp);
  if (res < 0)
    return NULL;

  return StaticPlugin_XMLRecipeLoader_ExitFunc;
}

bp::StaticPlugin
StaticPlugin_XMLRecipeLoader(StaticPlugin_XMLRecipeLoader_InitPlugin);

