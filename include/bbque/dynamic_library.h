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

#ifndef BBQUE_DYNAMIC_LIBRARY_H_
#define BBQUE_DYNAMIC_LIBRARY_H_

#include <string>

namespace bbque { namespace plugins {

/**
 * @brief A convenience class to manage shared library
 *
 * This class provides a set of basic services to load a shared library
 * and release them when not more needed.
 */
class DynamicLibrary {

public:

	/**
	 * @brief   Load the specified library
	 * @param   path the filesyste path of the shared object library to load
	 * @param	errorString a string to contain eventually load error messages
	 * @return  a pointer to the loaded library, NULL on errors
	 */
	static DynamicLibrary * Load(const std::string & path,
								std::string &errorString);
	/**
	 * @brief   Release this dynamic library loader
	 */
	~DynamicLibrary();

	/**
	 * @brief   Get a pointer to the specified library symbol
	 * @param   name the symbol name to lookup
	 * @return  a valid pointer to the requested symbol, NULL on errrors
	 */
	void * GetSymbol(const std::string & name);

private:

	/**
	 * @brief   Build a new instance of the dynamic library loader
	 */
	DynamicLibrary();

	/**
	 * @brief   Clone the dynamic library loader
	 * @param   handle the loader to clone
	 */
	DynamicLibrary(void * handle);

private:

	/**
	 * A pointer to the dynamic library represented by this object
	 */
	void * handle;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_DYNAMIC_LIBRARY_H_
