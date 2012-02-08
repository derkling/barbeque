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

#ifndef BBQUE_OBJECT_ADAPTER_H_
#define BBQUE_OBJECT_ADAPTER_H_

#include "bbque/plugins/plugin.h"

namespace bbque { namespace plugins {

/**
 * @brief Interface to adapt C plugin objects to C++ plugin objects.
 *
 * It must be passed to the PluginManager::createObject() function.
 */
struct ObjectAdapterIF {

	virtual ~ObjectAdapterIF() {}

	virtual void * adapt(void * object, PF_DestroyFunc df) = 0;
};


/**
 * @brief An Object adapter
 *
 * This template should be used if the object model implements the dual C/C++
 * object design pattern. Otherwise you need to provide your own object
 * adapter class that implements ObjectAdapterIF
 */
template<typename T, typename U>
struct ObjectAdapter : public ObjectAdapterIF {
	virtual void * adapt(void * object, PF_DestroyFunc df) {
		return new T((U *)object, df);
	}
};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_OBJECT_ADAPTER_H_
