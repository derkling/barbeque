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

#include "dummy_test.h"

#include <iostream>

namespace bbque { namespace plugins {

DummyTest::DummyTest() {
	std::cout << "DummyTest: Build new " PLUGIN_TYPE " object ["
		<< this << "]" << std::endl;
}

DummyTest::~DummyTest() {

}

//----- dummy module interface

void DummyTest::Test(void) {
	std::cout << "DummyTest: This is just a (working) " PLUGIN_TYPE " Module ["
		<< this << "]" << std::endl;
}

//----- static plugin interface

void * DummyTest::Create(PF_ObjectParams *) {
	return new DummyTest();
}

int32_t DummyTest::Destroy(void * plugin) {
  if (!plugin)
    return -1;
  delete (DummyTest *)plugin;
  return 0;
}

} // namesapce plugins

} // namespace bque

