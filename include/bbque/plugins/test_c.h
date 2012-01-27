/**
 *       @file  test_c.h
 *      @brief  The C object model for barbeque Testing plugins
 *
 * This defines the interface for Test barbque C based plugin. This file
 * provided the C based object model for Test plugins.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
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

#ifndef BBQUE_TEST_C_H_
#define BBQUE_TEST_C_H_

//----- TestModule C interface
typedef struct C_TestHandle_ { char c; } * C_TestHandle;
typedef struct C_Test_ {
	void (*Test)();
	C_TestHandle handle;
} C_Test;

#endif // BBQUE_TEST_C_H_

