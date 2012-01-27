/**
 *       @file  static_plugin.h
 *      @brief  A static plugin wrapper class
 *
 * This class provides a simple mechanism that lets static plugins register
 * their objects automatically with the PluginManager without requiring the
 * application to explicitly initialize them. The way it works (when it
 * works) is that the plugin defines a global instance of the StaticPlugin
 * and passes it its initialization function (with a signature that matches
 * PF_InitFunc). The StaticPlugin simply calls the
 * PluginManager::InitializePlugin() method that ignites the static plugin
 * initialization just like with dynamic plugins after loading the dynamic
 * library.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  01/27/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef BBQUE_STATIC_PLUGIN_H_
#define BBQUE_STATIC_PLUGIN_H_

#include "bbque/plugin_manager.h"

namespace bbque { namespace plugins {

class StaticPlugin {
public:
  StaticPlugin(PF_InitFunc initFunc) {
    PluginManager::InitializePlugin(initFunc);
  }
};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_STATIC_PLUGIN_H_

