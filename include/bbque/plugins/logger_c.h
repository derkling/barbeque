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

#ifndef BBQUE_LOGGER_C_H_
#define BBQUE_LOGGER_C_H_

//----- Logger C interface
typedef struct C_LoggerHandle_ { char c; } * C_LoggerHandle;

/**
 * @brief A C coded Logger
 *
 * A C based object model for LoggerIF plugins.
 */
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
