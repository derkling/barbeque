/**
 *       @file  console_logger.h
 *      @biief  A (dummy) console based logger
 *
 * This defines a console based logger to be used for logging if a more
 * advanced logger module is not available
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

#ifndef BBQUE_CONSOLE_LOGGER_H_
#define BBQUE_CONSOLE_LOGGER_H_

#include "bbque/plugins/logger.h"

#include <memory>
#include <cstdint>

namespace bbque { namespace plugins {

/**
 * @class Log4CppLogger
 * @brief The basic class for each Barbeque component
 */
class ConsoleLogger : public LoggerIF {

public:

	/**
	 * Return a singleton instance of the console logger
	 */
	static std::shared_ptr<ConsoleLogger> GetInstance(void);

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	~ConsoleLogger();

//----- Logger module interface

	/**
	 * \brief Send a log message with the priority DEBUG
	 * \param message the message to log
	 */
	void Debug(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority INFO
	 * \param message the message to log
	 */
	void Info(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority NOTICE
	 * \param message the message to log
	 */
	void Notice(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority WARN
	 * \param message the message to log
	 */
	void Warn(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority ERROR
	 * \param message the message to log
	 */
	void Error(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority CRIT
	 * \param message the message to log
	 */
	void Crit(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority ALERT
	 * \param message the message to log
	 */
	void Alert(const char *fmt, ...);

	/**
	 * \brief Send a log message with the priority FATAL
	 * \param message the message to log
	 */
	void Fatal(const char *fmt, ...);

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_CONSOLE_LOGGER_H_

