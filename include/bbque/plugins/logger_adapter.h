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

#ifndef BBQUE_LOGGER_ADAPTER_H_
#define BBQUE_LOGGER_ADAPTER_H_

#include "bbque/plugins/logger.h"
#include "bbque/plugins/logger_c.h"
#include "bbque/plugins/plugin.h"

#include <stdarg.h>

#define LOG_MAX_SENTENCE 128

namespace bbque { namespace plugins {

/**
 * @brief A C adapter for a Logger
 *
 * This class provides a wrapper to adapt C-coded LoggerIF modules.
 */
class LoggerAdapter : public LoggerIF {

public:

	LoggerAdapter(C_Logger * _logger, PF_DestroyFunc _df) :
		logger(_logger),
		df(_df) {
	}

	~LoggerAdapter() {
		if (df)
			df(logger);
	}

	// NOTE: Adapting a C plugins that way could incur a significatn overhead.
	// Since we don't use C based logger, we postpone a deeper analysis in
	// overheads and the identification of a better solution.
	void Debug(const char *fmt, ...) {
		va_list args;
		char str[LOG_MAX_SENTENCE];
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger->Debug(str);
	}
	void Info(const char *fmt, ...) {
		va_list args;
		char str[LOG_MAX_SENTENCE];
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger->Info(str);
	}
	void Notice(const char *fmt, ...) {
		va_list args;
		char str[LOG_MAX_SENTENCE];
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger->Debug(str);
		logger->Notice(str);
	}
	void Warn(const char *fmt, ...) {
		va_list args;
		char str[LOG_MAX_SENTENCE];
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger->Debug(str);
		logger->Warn(str);
	}
	void Error(const char *fmt, ...) {
		va_list args;
		char str[LOG_MAX_SENTENCE];
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger->Debug(str);
		logger->Error(str);
	}
	void Crit(const char *fmt, ...) {
		va_list args;
		char str[LOG_MAX_SENTENCE];
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger->Debug(str);
		logger->Crit(str);
	}
	void Alert(const char *fmt, ...) {
		va_list args;
		char str[LOG_MAX_SENTENCE];
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger->Debug(str);
		logger->Alert(str);
	}
	void Fatal(const char *fmt, ...) {
		va_list args;
		char str[LOG_MAX_SENTENCE];
		va_start(args, fmt);
		vsnprintf(str, LOG_MAX_SENTENCE, fmt, args);
		va_end(args);
		logger->Debug(str);
		logger->Fatal(str);
	}

private:

	C_Logger * logger;
	PF_DestroyFunc df;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_LOGGER_ADAPTER_H_
