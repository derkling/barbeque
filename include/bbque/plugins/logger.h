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

#ifndef BBQUE_LOGGER_H_
#define BBQUE_LOGGER_H_

#include "bbque/config.h"

#define LOGGER_NAMESPACE "logger."

#include <string>

/**
 * Prepend file and line number to the logMessage
 */
#define FORMAT_DEBUG(fmt) "%25s:%05d - " fmt, __FILE__, __LINE__

namespace bbque { namespace plugins {

/**
 * @brief The basic class for each Barbeque component
 *
 * This defines the basic logging services which are provided to each Barbeque
 * components. The object class defines logging and modules name.
 */
class LoggerIF {

public:

//----- Objects initialization data

	typedef enum Priority {
		DEBUG,
		INFO,
		NOTICE,
		WARN,
		ERROR,
		CRIT,
		ALERT,
		FATAL
	} Priority;

	class Configuration {
	public:
		Configuration(const char * cat, Priority prio = WARN) :
			category(cat),
			default_prio(prio) {};

		char const * category;
		Priority default_prio;

	};

//----- Objects interface

#ifdef BBQUE_DEBUG
	/**
	 * \brief Send a log message with the priority DEBUG
	 * \param fmt the message to log
	 */
	virtual void Debug(const char *fmt, ...) = 0;
#else
	void Debug(const char *fmt, ...) {(void)fmt;};
#endif
	/**
	 * \brief Send a log message with the priority INFO
	 * \param fmt the message to log
	 */
	virtual void Info(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority NOTICE
	 * \param fmt the message to log
	 */
	virtual void Notice(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority WARN
	 * \param fmt the message to log
	 */
	virtual void Warn(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority ERROR
	 * \param fmt the message to log
	 */
	virtual void Error(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority CRIT
	 * \param fmt the message to log
	 */
	virtual void Crit(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority ALERT
	 * \param fmt the message to log
	 */
	virtual void Alert(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority FATAL
	 * \param fmt the message to log
	 */
	virtual void Fatal(const char *fmt, ...) = 0;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_LOGGER_H_
