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

