/**
 *       @file  object.h
 *      @brief  The basic class for all Barbeque RTRM components
 *
 * This defines the basic class providing common supports for all Barbeque
 * components. The object class defines loging and modules name.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
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

#ifndef BBQUE_OBJECT_H_
#define BBQUE_OBJECT_H_

#include <memory>

#include "bbque/logger.h"
#include "bbque/exception.h"

#define THROW throw bbque::exception(__FILE__, __LINE__)

#define CHECK(condition) if (!(condition)) \
	THROW << "CHECK FAILED: '" << #condition << "'"

#ifdef BBQUE_DEBUG

# define ASSERT(condition) if (!(condition)) \
	THROW << "ASSERT FAILED: '" << #condition << "'"
#else
# define ASSERT(condition) {}
#endif // BBQUE_DEBUG

namespace bbque {

class Object {

public:

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	~Object();

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	const std::string & GetName() const {
		return name;
	}

protected:

	/**
	 * 
	 */
	std::unique_ptr<Logger> logger;

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	Object(std::string const & name);

private:

	/**
	 * 
	 */
	const std::string name;

}; // class Object

} // namespace bbque

#endif // BBQUE_OBJECT_H_

