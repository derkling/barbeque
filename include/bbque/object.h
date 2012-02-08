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
 * @brief The basic class from witch all Barbeque modules should derive
 *
 * This defines the basic class providing common supports for all Barbeque
 * components. The object class defines loging and modules name.
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
