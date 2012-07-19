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
  res = params->RegisterObject((const char *)MODULE_NAMESPACE, &rp);
  if (res < 0)
    return NULL;

  return StaticPlugin_XMLRecipeLoader_ExitFunc;
}

bp::StaticPlugin
StaticPlugin_XMLRecipeLoader(StaticPlugin_XMLRecipeLoader_InitPlugin);

