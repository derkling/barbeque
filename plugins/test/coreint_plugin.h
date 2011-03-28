/**
 *       @file  coreint_plugin.h
 *      @brief  CoreInteractionsTest plugin init and exit functions
 *
 * This defines a static C++ plugin which instantiate an object implementing
 * the TestIF interface.
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

#ifndef BBQUE_COREINT_PLUGIN_H_
#define BBQUE_COREINT_PLUGIN_H_

#include <cstdint>

#include "bbque/plugins/plugin.h"

extern "C" int32_t PF_exitFunc();
extern "C" PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params);

#endif // BBQUE_COREINT_PLUGIN_H_

