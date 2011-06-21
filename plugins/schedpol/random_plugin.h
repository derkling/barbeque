/**
 *       @file  yamca_plugin.h
 *      @brief  The Random resource scheduler (dynamic plugin)
 *
 * This defines a dynamic C++ plugin which implements the Random resource
 * scheduler heuristic.
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
 * =============================================================================
 */

#ifndef BBQUE_RANDOM_PLUGIN_H_
#define BBQUE_RANDOM_PLUGIN_H_

#include <cstdint>

#include "bbque/plugins/plugin.h"

extern "C" int32_t PF_exitFunc();
extern "C" PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params);

#endif // BBQUE_RANDOM_PLUGIN_H_
