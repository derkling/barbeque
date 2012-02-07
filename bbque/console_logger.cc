/**
 *       @file  console_logger.h
 *      @brief  A (dummy) console based logger
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

#include "bbque/console_logger.h"

#include <cstdarg>

#define LOG_MAX_SENTENCE 256

namespace bbque { namespace plugins {


std::shared_ptr<ConsoleLogger> ConsoleLogger::GetInstance(void) {
	static std::shared_ptr<ConsoleLogger> logger;

	if (logger)
		return logger;

	logger = std::shared_ptr<ConsoleLogger>(new ConsoleLogger);
	return logger;
}

ConsoleLogger::~ConsoleLogger() {
}

//----- Logger plugin interface

void ConsoleLogger::Debug(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	fprintf(stderr, "[DBG] %s\n", str);
}

void ConsoleLogger::Info(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	fprintf(stderr, "[INF] %s\n", str);
}

void ConsoleLogger::Notice(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	fprintf(stderr, "[NOT] %s\n", str);
}

void ConsoleLogger::Warn(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	fprintf(stderr, "[WRN] %s\n", str);
}

void ConsoleLogger::Error(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	fprintf(stderr, "[ERR] %s\n", str);
}

void ConsoleLogger::Crit(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	fprintf(stderr, "[CRT] %s\n", str);
}

void ConsoleLogger::Alert(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	fprintf(stderr, "[ALR] %s\n", str);
}

void ConsoleLogger::Fatal(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	fprintf(stderr, "[FAT] %s\n", str);
}

} // namespace plugins

} // namespace bbque

