/**
 *       @file  logger_c.h
 *      @brief  The C object model for barbeque Logger plugins
 *
 * This defines the interface for Logger barbque C based plugin. This file
 * provided the C based object model for Logger plugins.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  01/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef BBQUE_LOGGER_C_H_
#define BBQUE_LOGGER_C_H_

//----- Logger C interface
typedef struct C_LoggerHandle_ { char c; } * C_LoggerHandle;
typedef struct C_Logger_ {
	void (*Debug)	(const char *fmt, ...);
	void (*Info)	(const char *fmt, ...);
	void (*Notice)	(const char *fmt, ...);
	void (*Warn)	(const char *fmt, ...);
	void (*Error)	(const char *fmt, ...);
	void (*Crit)	(const char *fmt, ...);
	void (*Alert)	(const char *fmt, ...);
	void (*Fatal)	(const char *fmt, ...);
	C_LoggerHandle handle;
} C_Logger;

#endif // BBQUE_LOGGER_C_H_

