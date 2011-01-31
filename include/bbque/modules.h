/**
 *       @file  modules.h
 *      @brief  The object model for each barbque plugin.
 *
 * This defines the interface of all barbque plugin, which identify the object
 * model of the tool. Each such defined module could be defined as a plugin,
 * either statically linked or dynamically loaded.
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

#ifndef BBQUE_MODULES_H_
#define BBQUE_MODULES_H_

namespace bbque {

/**
 * @class TestModuleIF
 * @brief A module to support testing of other components implementation.
 */
class TestModuleIF {
	virtual void Test() = 0;
};



} // namespace bbque

#endif // BBQUE_MODULES_H_

