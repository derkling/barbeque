/**
 *       @file  logger.h
 *      @brief  The interface for the Barbeque logging sub-system
 *
 * This defines the basic logging services which are provided to each Barbeque
 * components. The object class defines logging and modules name.
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

#ifndef BBQUE_LOGGER_H_
#define BBQUE_LOGGER_H_

#define LOGGER_NAMESPACE "logger."

#include <string>

/**
 * Prepend file and line number to the logMessage
 */
#define FORMAT_DEBUG(fmt) "%25s:%05d - " fmt, __FILE__, __LINE__

namespace bbque { namespace plugins {

/**
 * @class Object
 * @brief The basic class for each Barbeque component
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

	/**
	 * \brief Send a log message with the priority DEBUG
	 * \param message the message to log
	 */
	virtual void Debug(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority INFO
	 * \param message the message to log
	 */
	virtual void Info(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority NOTICE
	 * \param message the message to log
	 */
	virtual void Notice(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority WARN
	 * \param message the message to log
	 */
	virtual void Warn(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority ERROR
	 * \param message the message to log
	 */
	virtual void Error(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority CRIT
	 * \param message the message to log
	 */
	virtual void Crit(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority ALERT
	 * \param message the message to log
	 */
	virtual void Alert(const char *fmt, ...) = 0;

	/**
	 * \brief Send a log message with the priority FATAL
	 * \param message the message to log
	 */
	virtual void Fatal(const char *fmt, ...) = 0;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_LOGGER_H_

