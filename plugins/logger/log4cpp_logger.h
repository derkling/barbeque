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

#ifndef BBQUE_LOG4CPP_LOGGER_H_
#define BBQUE_LOG4CPP_LOGGER_H_

#include "bbque/plugins/logger.h"
#include "bbque/plugins/plugin.h"

#include "bbque/config.h"

#include <cstdint>
#include <log4cpp/Category.hh>

#define MODULE_NAMESPACE LOGGER_NAMESPACE".log4cpp"
#define MODULE_CONFIG LOGGER_CONFIG".log4cpp"

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

/**
 * @brief The basic class for each Barbeque component
 *
 * This defines a Log4CPP based Logger plugin.
 */
class Log4CppLogger : public LoggerIF {

public:

//----- static plugin interface

	/**
	 *
	 */
	static void * Create(PF_ObjectParams * params);

	/**
	 *
	 */
	static int32_t Destroy(void * logger);

	/**
	 *
	 */
	virtual ~Log4CppLogger();

//----- Logger module interface

#ifdef BBQUE_DEBUG
	/**
	 * \brief Send a log message with the priority DEBUG
	 * \param fmt the message to log
	 */
	void Debug(const char *fmt, ...);
#endif

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


private:

	/**
	 * Set true to use colors for logging
	 */
	bool use_colors;

	/**
	 * Set true when the logger has been configured.
	 * This is done by parsing a configuration file the first time a Logger is created.
	 */
	static bool configured;

	/**
	 * @brief The logger reference
	 * Use this logger reference, related to the 'log' category, to log your messages
	 */
	log4cpp::Category & logger;

	/**
	 * @brief Build a new Barbeque component
	 * Each Barbeque component is associated to a logger category whose
	 * name is prefixed by "bbque."
	 * @param logName the log category, this name is (forcely) prepended by the
	 * 	class namespace "bbque."
	 */
	Log4CppLogger(char const * category);


	/**
	 * @brief   Load Logger configuration
	 * @return  true if the configuration has been properly loaded and object
	 * could be built, false otherwise
	 */
	static bool Configure(PF_ObjectParams * params);

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_LOG4CPP_LOGGER_H_
