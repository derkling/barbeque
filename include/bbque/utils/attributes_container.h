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

#ifndef BBQUE_ATTRIBUTES_H_
#define BBQUE_ATTRIBUTES_H_

#include <map>
#include <memory>
#include <string>


namespace bbque { namespace utils {


/**
 * @brief A coontainer of module-specific attributes
 *
 * The class provide an interface for setting and getting specific attributes.
 * We expect to use this class for extending classes as Recipe, Application
 * and WorkingMode, in order to manage information specific for each plugin in
 * a flexible way.
 *
 * Considering a scheduler module, the extention provided by this class could
 * be exploit to append information to the Application descriptor. For
 * instance how many times it has been re-scheduled, or in the WorkingMode
 * descriptor (e.g. How much that working mode is "good" for the scheduling).
 */
class AttributesContainer {

public:

	/**
	 * struct Attribute
	 *
	 * The structure must be inherited in order to implement a specific
	 * attribute to manage. This provides just a string to classify the
	 * attribute, and the identifying key string.
	 */
	typedef struct Attribute {
		/** Namespace */
		std::string ns;
		/** ID key  */
		std::string key;
		/** Constructor */
		Attribute(std::string const & _ns, std::string const & _key):
			ns(_ns),
			key(_key) {}
	} Attribute_t;

	/** Shared pointer to Attribute_t */
	typedef std::shared_ptr<Attribute_t> AttrPtr_t;

	/** Data type for the structure storing the whole set of attributes */
	typedef std::multimap<std::string, AttrPtr_t> AttributesMap_t;

	/**
	 * @brief ExitCode_t
	 *
	 * Exit codes used in the class methods
	 */
	typedef enum ExitCode {
		/** Success return code */
		ATTR_OK = 0,
		/** Generic error code */
		ATTR_ERR
	} ExitCode_t;

	/**
	 * @brief Constructor
	 */
	AttributesContainer();

	/**
	 * @brief Destructor
	 */
	virtual ~AttributesContainer();

	/**
	 * @brief Set a specific attribute
	 *
	 * @param attr Shared pointer to the Attribute object (commonly a derived
	 * class)
	 *
	 * @return ATTR_OK for success, ATTR_ERR otherwise
	 * @note The current implementation does not expect to fail the insertion
	 */
	ExitCode_t SetAttribute(AttrPtr_t attr);

	/**
	 * @brief Get a plugin specific data
	 *
	 * @param ns The namespace of the attribute
	 * @param key The ID key of the attribute
	 *
	 * @return A shared pointer to the Attribute object
	 */
	AttrPtr_t GetAttribute(std::string const & ns, std::string const & key);

	/**
	 * @brief Clear an attribute or a set
	 *
	 * Remove all the attributes under a given namespace or a specific one if
	 * the key is specified.
	 *
	 * @param ns The attribute namespace
	 * @param key The attribute key
	 */
	void ClearAttribute(std::string const & ns, std::string const & key = "");

protected:

	/**
	 * @brief Multi-map storing the attributes
	 *
	 * The namespace is a first level key for the multi-map. The value is an
	 * AttrPtr_t storing the ID key for the specific attribute. Users should
	 * exploit a properly extension of the Attribute class in order to store
	 * the data.
	 */
	AttributesMap_t attributes;
};

} // namespace utils

} // namespace bbque

#endif // BBQUE_ATTRIBUTES_H_
