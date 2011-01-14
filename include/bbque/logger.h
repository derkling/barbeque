/**
 *       @file  logger.h
 *      @brief  The interface fot the Barbeque logging sub-system
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

#include "bbque/barbeque.h"

#include <memory>
#include <string>

/**
 * Prepend file and line number to the logMessage
 */
#define FORMAT_DEBUG(fmt) "%25s:%05d - " fmt, __FILE__, __LINE__

namespace bbque {

/**
 * @class Object
 * @brief The basic class for each Barbeque component
 */
class Logger {

public:

	static std::unique_ptr<Logger> GetInstance(std::string const & name = "undef");

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
	 * @brief   
	 * @param   
	 * @return  
	 */
	virtual void Fatal(const char *fmt, ...) = 0;

protected:

	/**
	 * @brief The logger reference
	 * Use this logger reference, related to the 'log' category, to log your messages
	 */
	const std::string log_name;

	std::unique_ptr<Logger> instance;

	/**
	 * @brief Build a new Barbeque component
	 *
	 * Each Barbeque component is associated to a logger category whose
	 * name is prefixed by "bbque."
	 * @param logName the log category, this name is (forcely) prepended by the
	 * 	class namespace "bbque."
	 */
	Logger(std::string const & name);

};

} // namespace bbque

#endif // BBQUE_LOGGER_H

