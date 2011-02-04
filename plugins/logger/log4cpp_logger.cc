/**
 *       @file  log4cpp.cpp
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

#include "log4cpp_logger.h"

#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PropertyConfigurator.hh>

using log4cpp::Category;
using log4cpp::Priority;
using log4cpp::PropertyConfigurator;

#define LOG_MAX_SENTENCE 256

#define COLOR_WHITE  "\033[1;37m%s\033[0m"
#define COLOR_LGRAY  "\033[37m%s\033[0m"
#define COLOR_GRAY   "\033[1;30m%s\033[0m"
#define COLOR_BLACK  "\033[30m%s\033[0m"
#define COLOR_RED    "\033[31m%s\033[0m"
#define COLOR_LRED   "\033[1;31m%s\033[0m"
#define COLOR_GREEN  "\033[32m%s\033[0m"
#define COLOR_LGREEN "\033[1;32m%s\033[0m"
#define COLOR_BROWN  "\033[33m%s\033[0m"
#define COLOR_YELLOW "\033[1;33m%s\033[0m"
#define COLOR_BLUE   "\033[34m%s\033[0m"
#define COLOR_LBLUE  "\033[1;34m%s\033[0m"
#define COLOR_PURPLE "\033[35m%s\033[0m"
#define COLOR_PINK   "\033[1;35m%s\033[0m"
#define COLOR_CYAN   "\033[36m%s\033[0m"
#define COLOR_LCYAN  "\033[1;36m%s\033[0m"

#define COLOR_INFO		COLOR_LGREEN
#define COLOR_NOTICE	COLOR_GREEN
#define COLOR_WARN		COLOR_YELLOW
#define COLOR_ERROR		COLOR_PURPLE
#define COLOR_CRIT		COLOR_PURPLE
#define COLOR_ALERT		COLOR_LRED
#define COLOR_FATAL		COLOR_RED

namespace bbque { namespace plugins {

bool Log4CppLogger::configured = false;

Log4CppLogger::Log4CppLogger(char const * category) :
	use_colors(true),
	logger(Category::getInstance(category)) {
}

Log4CppLogger::~Log4CppLogger() {
}

//----- static plugin interface

void * Log4CppLogger::Create(PF_ObjectParams * params) {
	LoggerIF::Configuration * conf = (LoggerIF::Configuration*) params->data;

	if (!Configure(params))
		return NULL;

	return new Log4CppLogger(conf->category);
}

int32_t Log4CppLogger::Destroy(void * plugin) {
	if (!plugin)
		return -1;
	delete (Log4CppLogger *)plugin;
	return 0;
}

bool Log4CppLogger::Configure(PF_ObjectParams * params) {

	if (configured)
		return true;

	// Setting up Appender, layout and Category
	try {
		log4cpp::PropertyConfigurator::configure("/tmp/bbque.conf");
		configured = true;
	} catch (log4cpp::ConfigureFailure e) {
		std::cerr << "Log4CPP configuration error: " << e.what() << std::endl;
		return NULL;
	}

	return true;

}

//----- Logger plugin interface

void Log4CppLogger::Debug(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isDebugEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger.log(log4cpp::Priority::DEBUG, str);
	}
}

void Log4CppLogger::Info(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isInfoEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		if (use_colors)
			logger.log(log4cpp::Priority::INFO,
					COLOR_INFO, str);
		else
			logger.log(log4cpp::Priority::INFO, str);
	}

}

void Log4CppLogger::Notice(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isNoticeEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		if (use_colors)
			logger.log(log4cpp::Priority::NOTICE,
					COLOR_NOTICE, str);
		else
			logger.log(log4cpp::Priority::NOTICE, str);
	}
}

void Log4CppLogger::Warn(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isWarnEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		if (use_colors)
			logger.log(log4cpp::Priority::WARN,
					COLOR_WARN, str);
		else
			logger.log(log4cpp::Priority::WARN, str);
	}
}

void Log4CppLogger::Error(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isErrorEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		if (use_colors)
			logger.log(log4cpp::Priority::ERROR,
					COLOR_ERROR, str);
		else
			logger.log(log4cpp::Priority::ERROR, str);
	}
}

void Log4CppLogger::Crit(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isCritEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		if (use_colors)
			logger.log(log4cpp::Priority::CRIT,
					COLOR_CRIT, str);
		else
			logger.log(log4cpp::Priority::CRIT, str);
	}
}

void Log4CppLogger::Alert(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isAlertEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		if (use_colors)
			logger.log(log4cpp::Priority::ALERT,
					COLOR_ALERT, str);
		else
			logger.log(log4cpp::Priority::ALERT, str);
	}
}

void Log4CppLogger::Fatal(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isFatalEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		if (use_colors)
			logger.log(log4cpp::Priority::FATAL,
					COLOR_FATAL, str);
		else
			logger.log(log4cpp::Priority::FATAL, str);
	}
}

} // namespace plugins

} // namespace bbque

