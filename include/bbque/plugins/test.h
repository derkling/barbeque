/**
 *       @file  test.h
 *      @brief  The interface for Barbeque Testing sub-syste
 *
 * This defines the common interface for each testing module which can be
 * developed for the Barbeque framework.
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

#ifndef BBQUE_TEST_H_
#define BBQUE_TEST_H_

#define TEST_NAMESPACE "test."

namespace bbque { namespace plugins {

/**
 * @class TestModuleIF
 * @brief A module to support testing of other components implementation.
 */
class TestIF {

public:

	virtual void Test() = 0;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_TEST_H_

