/**
 *       @file  static_plugin.h
 *      @brief  An example of static C++ plugin
 *
 * This defines a simple example of static C++ plugin which is intended both to
 * demostrate how to write them and to test the PluginManager implementation.
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
 * =====================================================================================
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
 * @class DummyTest
 * @brief A simple object registered as a static C++ plugin.
 */
class DummyTest : public TestIF {

public:

//----- static plugin interface

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	static void * Create(PF_ObjectParams *);

	/**
	 * @brief   
	 * @param   
	 * @return  
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

