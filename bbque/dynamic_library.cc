/**
 *       @file  dynamic_library.cc
 *      @brief  A generic class to manage shared libraries
 *
 * This class provides a generic interface to manage shared libraries.
 *
 * Detailed description starts here.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/27/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include "bbque/dynamic_library.h"

#include <dlfcn.h>
#include <sstream>
#include <iostream>

#include "bbque/utils/utility.h"
#define FMT(fmt) BBQUE_FMT(COLOR_LGREEN, "DL", fmt)

namespace bbque { namespace plugins {

DynamicLibrary::DynamicLibrary(void * _handle) :
	handle(_handle) {
	}

DynamicLibrary::~DynamicLibrary() {
	if (handle) {
		::dlclose(handle);
	}
}

DynamicLibrary * DynamicLibrary::Load(const std::string & name,
				std::string & errorString) {
	void * handle = NULL;

	if (name.empty()) {
		errorString = "Empty path";
		goto err_load;
	}

	handle = ::dlopen(name.c_str(), RTLD_NOW);
	if (!handle) {
		std::string dlErrorString;
		const char *zErrorString = ::dlerror();
		if (zErrorString)
			errorString = zErrorString;
		else
			errorString = std::string("Undef");

		goto err_load;
	}

	return new DynamicLibrary(handle);

err_load:
	fprintf(stderr, FMT("FAILED loading [%s], Error:\n%s\n"),
			name.c_str(), errorString.c_str());
	return NULL;
}

void * DynamicLibrary::GetSymbol(const std::string & symbol) {
	if (!handle)
		return NULL;

	return ::dlsym(handle, symbol.c_str());
}

} // namespace plugins

} // namespace bbque

