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

#ifndef BBQUE_STATIC_PLUGIN_H_
#define BBQUE_STATIC_PLUGIN_H_

#include "bbque/plugin_manager.h"

namespace bbque { namespace plugins {

/**
 * @brief  A static plugin wrapper class
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
 */
class StaticPlugin {
public:
  StaticPlugin(PF_InitFunc initFunc) {
    PluginManager::InitializePlugin(initFunc);
  }
};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_STATIC_PLUGIN_H_
