/**
 *       @file  static_plugin.cc
 *      @brief  An example of static C++ plugin
 *
 * This defines a simple example of static C++ plugin which is intended both to
 * demostrate how to write them and to test the PluginManager implementation.
 *
 * Detailed description starts here.
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

#include "static_test_object.h"

namespace bbque { namespace plugins {

TestingObject::TestingObject() {

}

TestingObject::~TestingObject() {

}

//----- testing plugin interface

void TestingObject::Test(void) {

}

//----- static plugin interface

void * TestingObject::Create(PF_ObjectParams *) {
  return new TestingObject();
}

int32_t TestingObject::Destroy(void * plugin) {
  if (!plugin)
    return -1;
  delete (TestingObject *)plugin;
  return 0;
}




} // namesapce plugins

} // namespace bque

