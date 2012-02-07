/**
 *       @file  log4cpp_logger.cc
 *      @brief  A logger plugin based on Log4Cpp library
 *
 * Detailed description here.
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

#include "log4cpp_logger.h"

#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PropertyConfigurator.hh>

namespace l4 = log4cpp;
namespace po = boost::program_options;

#define LOG_MAX_SENTENCE 256

#define LOG4CPP_COLOR_WHITE	"\033[1;37m%s\033[0m"
#define LOG4CPP_COLOR_LGRAY	"\033[37m%s\033[0m"
#define LOG4CPP_COLOR_GRAY	"\033[1;30m%s\033[0m"
#define LOG4CPP_COLOR_BLACK	"\033[30m%s\033[0m"
#define LOG4CPP_COLOR_RED	"\033[31m%s\033[0m"
#define LOG4CPP_COLOR_LRED	"\033[1;31m%s\033[0m"
#define LOG4CPP_COLOR_GREEN	"\033[32m%s\033[0m"
#define LOG4CPP_COLOR_LGREEN	"\033[1;32m%s\033[0m"
#define LOG4CPP_COLOR_BROWN	"\033[33m%s\033[0m"
#define LOG4CPP_COLOR_YELLOW	"\033[1;33m%s\033[0m"
#define LOG4CPP_COLOR_BLUE	"\033[34m%s\033[0m"
#define LOG4CPP_COLOR_LBLUE	"\033[1;34m%s\033[0m"
#define LOG4CPP_COLOR_PURPLE	"\033[35m%s\033[0m"
#define LOG4CPP_COLOR_PINK	"\033[1;35m%s\033[0m"
#define LOG4CPP_COLOR_CYAN	"\033[36m%s\033[0m"
#define LOG4CPP_COLOR_LCYAN	"\033[1;36m%s\033[0m"

#define LOG4CPP_COLOR_INFO	LOG4CPP_COLOR_LGREEN
#define LOG4CPP_COLOR_NOTICE	LOG4CPP_COLOR_GREEN
#define LOG4CPP_COLOR_WARN	LOG4CPP_COLOR_YELLOW
#define LOG4CPP_COLOR_ERROR	LOG4CPP_COLOR_PURPLE
#define LOG4CPP_COLOR_CRIT	LOG4CPP_COLOR_PURPLE
#define LOG4CPP_COLOR_ALERT	LOG4CPP_COLOR_LRED
#define LOG4CPP_COLOR_FATAL	LOG4CPP_COLOR_RED

namespace bbque { namespace plugins {

bool Log4CppLogger::configured = false;

Log4CppLogger::Log4CppLogger(char const * category) :
	use_colors(true),
	logger(l4::Category::getInstance(category)) {
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

	// Declare the supported options
	static std::string conf_file_path;
	po::options_description log4cpp_opts_desc("Log4CPP Options");
	log4cpp_opts_desc.add_options()
		(LOGGER_NAMESPACE"log4cpp.conf_file", po::value<std::string>
#ifdef BBQUE_DEBUG
		 (&conf_file_path)->default_value(
			 BBQUE_PATH_PREFIX"/"BBQUE_PATH_CONF"/bbque.conf_dbg"),
#else
		 (&conf_file_path)->default_value(
			 BBQUE_PATH_PREFIX"/"BBQUE_PATH_CONF"/bbque.conf"),
#endif
		 "configuration file path")
		;
	static po::variables_map log4cpp_opts_value;


	// Get configuration params
	PF_Service_ConfDataIn data_in;
	data_in.opts_desc = &log4cpp_opts_desc;
	PF_Service_ConfDataOut data_out;
	data_out.opts_value = &log4cpp_opts_value;
	PF_ServiceData sd;
	sd.id = LOGGER_NAMESPACE"log4cpp";
	sd.request = &data_in;
	sd.response = &data_out;

	int32_t response = params->platform_services->InvokeService(
			PF_SERVICE_CONF_DATA, sd);
	if (response!=PF_SERVICE_DONE)
		return NULL;

	fprintf(stdout, "Log4CppLogger: Using configuration file [%s]\n",
			conf_file_path.c_str());

	// Setting up Appender, layout and Category
	try {
		l4::PropertyConfigurator::configure(conf_file_path);
		configured = true;
	} catch (log4cpp::ConfigureFailure e) {
		fprintf(stdout, "Log4CppLogger: configuration error [%s]\n",
				e.what());
		return false;
	}

	return true;

}

//----- Logger plugin interface

#ifdef BBQUE_DEBUG
void Log4CppLogger::Debug(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isDebugEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger.log(l4::Priority::DEBUG, str);
	}
}
#endif

void Log4CppLogger::Info(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	if (logger.isInfoEnabled()) {
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		if (use_colors)
			logger.log(l4::Priority::INFO,
					LOG4CPP_COLOR_INFO, str);
		else
			logger.log(l4::Priority::INFO, str);
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
			logger.log(l4::Priority::NOTICE,
					LOG4CPP_COLOR_NOTICE, str);
		else
			logger.log(l4::Priority::NOTICE, str);
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
			logger.log(l4::Priority::WARN,
					LOG4CPP_COLOR_WARN, str);
		else
			logger.log(l4::Priority::WARN, str);
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
			logger.log(l4::Priority::ERROR,
					LOG4CPP_COLOR_ERROR, str);
		else
			logger.log(l4::Priority::ERROR, str);
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
			logger.log(l4::Priority::CRIT,
					LOG4CPP_COLOR_CRIT, str);
		else
			logger.log(l4::Priority::CRIT, str);
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
			logger.log(l4::Priority::ALERT,
					LOG4CPP_COLOR_ALERT, str);
		else
			logger.log(l4::Priority::ALERT, str);
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
			logger.log(l4::Priority::FATAL,
					LOG4CPP_COLOR_FATAL, str);
		else
			logger.log(l4::Priority::FATAL, str);
	}
}

} // namespace plugins

} // namespace bbque

