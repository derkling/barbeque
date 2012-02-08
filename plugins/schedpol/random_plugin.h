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

#ifndef BBQUE_RANDOM_PLUGIN_H_
#define BBQUE_RANDOM_PLUGIN_H_

#include <cstdint>

#include "bbque/plugins/plugin.h"

extern "C" int32_t PF_exitFunc();
extern "C" PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params);

#endif // BBQUE_RANDOM_PLUGIN_H_
