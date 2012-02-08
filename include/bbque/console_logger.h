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

#ifndef BBQUE_CONSOLE_LOGGER_H_
#define BBQUE_CONSOLE_LOGGER_H_

#include "bbque/plugins/logger.h"

#include <memory>
#include <cstdint>

namespace bbque { namespace plugins {

/**
 * @brief The basic class for each Barbeque component
 *
 * This defines a console based logger to be used for logging if a more
 * advanced logger module is not available
 */
class ConsoleLogger : public LoggerIF {

public:

	/**
	 * Return a singleton instance of the console logger
	 */
	static std::shared_ptr<ConsoleLogger> GetInstance(void);

	/**
	 *
	 */
	~ConsoleLogger();

//----- Logger module interface

	/**
	 * \brief Send a log message with the priority DEBUG
	 * \param fmt the message to log
	 */
	void Debug(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority INFO
	 * \param fmt the message to log
	 */
	void Info(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority NOTICE
	 * \param fmt the message to log
	 */
	void Notice(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority WARN
	 * \param fmt the message to log
	 */
	void Warn(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority ERROR
	 * \param fmt the message to log
	 */
	void Error(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority CRIT
	 * \param fmt the message to log
	 */
	void Crit(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority ALERT
	 * \param fmt the message to log
	 */
	void Alert(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority FATAL
	 * \param fmt the message to log
	 */
	void Fatal(const char *fmt, ...);

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_CONSOLE_LOGGER_H_
