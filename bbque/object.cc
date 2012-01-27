/**
 *       @file  object.cpp
 *      @brief  The basic class for all Barbeque RTRM components
 *
 * This implements the basic class providing common supports for all Barbeque
 * components. The object class defines loging and modules name.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  01/11/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include "bbque/object.h"

namespace bbque {

Object::Object(std::string const & name_) :
	name("bq."+name_)
{
	//logger = move(Logger::GetInstance(name));
}

Object::~Object()
{

}

} // namespace bbque

