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

#ifndef BBQUE_PLUGINS_TESTING_H_
#define BBQUE_PLUGINS_TESTING_H_

#include "bbque/plugins/test.h"
#include "bbque/plugins/plugin.h"

#include <cstdint>

#ifndef BBQUE_DYNAMIC_PLUGIN
# define DummyTest DummyTestS
# define	PLUGIN_TYPE "STATIC"
#else
# define DummyTest DummyTestD
# define	PLUGIN_TYPE "DYNAMIC"
#endif

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

/**
 * @brief A simple object registered as a static C++ plugin.
 *
 * This defines a simple example of static C++ plugin which is intended both to
 * demostrate how to write them and to test the PluginManager implementation.
 */
class DummyTest : public TestIF {

public:

//----- static plugin interface

	/**
	 *
	 */
	static void * Create(PF_ObjectParams *);

	/**
	 *
	 */
	static int32_t Destroy(void *);

	~DummyTest();

//----- dummy module interface

	virtual void Test();

private:

	/**
	 * @brief   The plugins constructor
	 * Plugins objects could be build only by using the "create" method.
	 * Usually the PluginManager acts as object
	 * @param   
	 * @return  
	 */
	DummyTest();

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_PLUGINS_TESTING_H_
