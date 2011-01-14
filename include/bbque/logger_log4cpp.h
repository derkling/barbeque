/**
 *       @file  log4cppLogger.h
 *      @brief  A logger plugin based on Log4Cpp library
 *
 * Detailed description here.
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

#ifndef BBQUE_OBJECT_H
#define BBQUE_OBJECT_H

#include "bbque/logger.h"

// Log4Cpp Logging Utilities
//#include <log4cpp/Portability.hh>
//#ifdef LOG4CPP_HAVE_UNISTD_H
//# include <unistd.h>
//#endif
#include <log4cpp/Category.hh>
//#include <log4cpp/Appender.hh>
//#include <log4cpp/FileAppender.hh>
//#include <log4cpp/OstreamAppender.hh>
//#ifdef LOG4CPP_HAVE_SYSLOG
//# include <log4cpp/SyslogAppender.hh>
//#endif
//#include <log4cpp/Layout.hh>
//#include <log4cpp/BasicLayout.hh>
//#include <log4cpp/Priority.hh>
//#include <log4cpp/NDC.hh>


namespace bbque {

/**
 * @class Log4CppLogger
 * @brief The basic class for each Barbeque component
 */
class Log4CppLogger : public Logger {

public:

	/**
	 * @brief Build a new Barbeque component
	 * Each Barbeque component is associated to a logger category whose
	 * name is prefixed by "bbque."
	 * @param logName the log category, this name is (forcely) prepended by the
	 * 	class namespace "bbque."
	 */
	Log4CppLogger(std::string const & name = "undef");


	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	~Log4CppLogger();

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


private:

	/**
	 * @brief The logger reference
	 * Use this logger reference, related to the 'log' category, to log your messages
	 */
	log4cpp::Category & l4cpp;

};

} // namespace bbque

#endif
