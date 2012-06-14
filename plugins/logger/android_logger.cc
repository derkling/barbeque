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

#include "android_logger.h"

#include <android/log.h>

#define LOG_MAX_SENTENCE 256

namespace bbque { namespace plugins {

AndroidLogger::AndroidLogger(char const * cat) :
	category(cat) {

}

AndroidLogger::~AndroidLogger() {
}

//----- static plugin interface

void * AndroidLogger::Create(PF_ObjectParams * params) {
	LoggerIF::Configuration * conf = (LoggerIF::Configuration*) params->data;
	return new AndroidLogger(conf->category);
}

int32_t AndroidLogger::Destroy(void * plugin) {
	if (!plugin)
		return -1;
	delete (AndroidLogger *)plugin;
	return 0;
}

//----- Logger plugin interface

#ifdef BBQUE_DEBUG
void AndroidLogger::Debug(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	__android_log_print(ANDROID_LOG_DEBUG, category.c_str(), str);
}
#endif

void AndroidLogger::Info(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	__android_log_print(ANDROID_LOG_VERBOSE, category.c_str(), str); 

}

void AndroidLogger::Notice(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	__android_log_print(ANDROID_LOG_INFO, category.c_str(), str); 
}

void AndroidLogger::Warn(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	__android_log_print(ANDROID_LOG_WARN, category.c_str(), str); 
}

void AndroidLogger::Error(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	__android_log_print(ANDROID_LOG_ERROR, category.c_str(), str); 
}

void AndroidLogger::Crit(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	__android_log_print(ANDROID_LOG_ERROR, category.c_str(), str); 
}

void AndroidLogger::Alert(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	__android_log_print(ANDROID_LOG_ERROR, category.c_str(), str); 
}

void AndroidLogger::Fatal(const char *fmt, ...) {
	va_list args;
	char str[LOG_MAX_SENTENCE];

	va_start(args, fmt);
	vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
	va_end(args);
	__android_log_print(ANDROID_LOG_FATAL, category.c_str(), str); 
}

} // namespace plugins

} // namespace bbque

