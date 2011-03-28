/**
 *       @file  xml_plugin.h
 *      @brief  Plugin management functions for XMLRecipeLoader (definitions)
 *
 * This define the init and exit functions for the plugin XMLRecipeLoader.
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

#ifndef BBQUE_PLUGIN_RLOADER_XML_
#define BBQUE_PLUGIN_RLOADER_XML_

#include <cstdint>

#include "bbque/plugins/plugin.h"

extern "C" int32_t StaticPlugin_XMLRecipeLoader_ExitFunc();
extern "C" PF_ExitFunc StaticPlugin_XMLRecipeLoader_InitPlugin(const PF_PlatformServices * params);

#endif // BBQUE_PLUGIN_RLOADER_XML_

