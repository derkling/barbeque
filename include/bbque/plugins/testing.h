/**
 *       @file  testing.h
 *      @brief  
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

#ifndef BBQUE_TESTING_OBJECT_H_
#define BBQUE_TESTING_OBJECT_H_

namespace bbque { namespace plugins {

class TestingObjectIF {
	virtual void Test() = 0;
};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_TESTING_OBJECT_H_

