/**
 *       @file  logger_adapter.h
 *      @brief  A C++ wrapper class for C based Logger plugins
 *
 * This class provides a wrapper to adapt Logger modules written in C.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  02/01/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_LOGGER_ADAPTER_H_
#define BBQUE_LOGGER_ADAPTER_H_

#include "bbque/plugins/logger.h"
#include "bbque/plugins/logger_c.h"
#include "bbque/plugins/plugin.h"

#include <stdarg.h>

#define LOG_MAX_SENTENCE 128

namespace bbque { namespace plugins {

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

