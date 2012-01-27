/**
 *       @file  object_adapter.h
 *      @brief  A C++ wrapper for C plugins
 *
 * This class provides a wrapper for C based plugins.
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

#ifndef BBQUE_OBJECT_ADAPTER_H_
#define BBQUE_OBJECT_ADAPTER_H_

#include "bbque/plugins/plugin.h"

namespace bbque { namespace plugins {

/**
 * @class ObjectAdapterIF
 * @brief This interface is used to adapt C plugin objects to C++ plugin objects.
 *
 * It must be passed to the PluginManager::createObject() function.
 */
struct ObjectAdapterIF {

	virtual ~ObjectAdapterIF() {}

	virtual void * adapt(void * object, PF_DestroyFunc df) = 0;
};


/**
 * @class ObjectAdapter
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

