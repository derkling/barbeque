/**
 *       @file  log4cpp_logger.h
 *      @brief  A Logger plugin based on Log4Cpp library
 *
 * This defines a Log4CPP based Logger plugin.
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

#ifndef BBQUE_LOG4CPP_LOGGER_H_
#define BBQUE_LOG4CPP_LOGGER_H_

#include "bbque/plugins/logger.h"
#include "bbque/plugins/plugin.h"

#include "bbque/config.h"

#include <cstdint>
#include <log4cpp/Category.hh>

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

/**
 * @class Log4CppLogger
 * @brief The basic class for each Barbeque component
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
	~Log4CppLogger();

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

