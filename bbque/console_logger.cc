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

