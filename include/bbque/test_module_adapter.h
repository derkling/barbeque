/**
 *       @file  test_module_adapter.h
 *      @brief  A C++ wrapper class for C based TestModules
 *
 * This class provides a wrapper to adapt Test modules written in C.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  02/01/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_TEST_MODULE_ADAPTER_H_
#define BBQUE_TEST_MODULE_ADAPTER_H_

#include "bbque/modules.h"
#include "bbque/plugins/plugin.h"

namespace bbque { namespace plugins {

class TestModuleAdapter : public TestModuleIF {

public:

	TestModuleAdapter(C_TestModule * _tm, PF_DestroyFunc _df) :
		tm(_tm),
		df(_df) {
	}

	~TestModuleAdapter() {
		if (df)
			df(tm);
	}

	void Test() {
		tm->Test();
	}

private:

	C_TestModule * tm;
	PF_DestroyFunc df;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_TEST_MODULE_ADAPTER_H_

