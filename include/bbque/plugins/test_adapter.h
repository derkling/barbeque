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

#ifndef BBQUE_TEST_ADAPTER_H_
#define BBQUE_TEST_ADAPTER_H_

#include "bbque/plugins/test.h"
#include "bbque/plugins/test_c.h"
#include "bbque/plugins/plugin.h"

namespace bbque { namespace plugins {

/**
 * @brief A C adapter for a Test
 *
 * This class provides a wrapper to adapt C-coded TestIF modules.
 */
class TestAdapter : public TestIF {

public:

	TestAdapter(C_Test * _test, PF_DestroyFunc _df) :
		test(_test),
		df(_df) {
	}

	~TestAdapter() {
		if (df)
			df(test);
	}

	void Test() {
		test->Test();
	}

private:

	C_Test * test;
	PF_DestroyFunc df;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_TEST_ADAPTER_H_
