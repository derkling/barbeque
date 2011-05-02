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

#include "bbque/plugins/logger.h"
#include "bbque/exception.h"

#define THROW throw bbque::Exception(__FILE__, __LINE__)

#define CHECK(condition) if (!(condition)) \
	THROW << "CHECK FAILED in " << __func__ << " @ " \
		<< __FILE__ << ":" << __LINE__ << "\n" \
		<< "  cond():  " << #condition << "\n"

#ifdef BBQUE_DEBUG
/**
 * Debugging support
 */
# define DEBUG(fmt, ...) \
	logger->Debug("%s@%s:%d - " fmt, \
			__func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
# define DEBUG(fmt, ...) do {} while (0)
#endif // BBQUE_DEBUG

namespace bbque {

/**
 * @class Object
 * @brief The basic class from witch all Barbeque modules should derive
 */
class Object {

public:

	/**
	 * @brief  Release this object
	 */
	~Object();

	/**
	 * @brief  Get the name of this object
	 * @return this object name
	 */
	const std::string & GetName() const {
		return name;
	}

protected:

	/**
	 * The pointer to logger of this object
	 */
	std::unique_ptr<plugins::LoggerIF> logger;

	/**
	 * @brief   Build a new object
	 * @param   name a name identitying this object
	 */
	Object(std::string const & name);

private:

	/**
	 * This object name
	 */
	const std::string name;

}; // class Object

} // namespace bbque

#endif // BBQUE_OBJECT_H_

