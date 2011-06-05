/**
 *       @file  plugin_data.h
 *      @brief  Class for managing plugin specific data
 *
 * This defines a class for managing plugin specific data.
 * The class provide an interface for setting and getting plugin specific
 * attributes. We expect to use this class for extending classes as
 * Application and WorkingMode.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  01/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_PLUGIN_DATA_H_
#define BBQUE_PLUGIN_DATA_H_

#include <map>
#include <memory>
#include <string>

typedef std::shared_ptr<void> VoidPtr_t;

namespace bbque { namespace app {

/** Pair key-value type */
typedef std::pair<std::string, VoidPtr_t> DataPair_t;
/** Map of 32-bit integer type data */
typedef std::multimap<std::string, DataPair_t> PlugDataMap_t;


/**
 * @class PluginsData
 *
 * The class provide an interface for setting and getting plugin specific
 * attributes. We expect to use this class for extending classes as Recipe,
 * Application and WorkingMode, in order to manage information specific for
 * each plugin in a flexible way.
 *
 * Considering a scheduler module, the extention provided by this class could
 * be exploit to append information to the Application descriptor. For
 * instance how many times it has been re-scheduled, or in the WorkingMode
 * descriptor (e.g. How much that working mode is "good" for the scheduling).
 */
class PluginsData {

public:

	/**
	 * @brief Constructor
	 */
	PluginsData();

	/**
	 * @brief Destructor
	 */
	virtual ~PluginsData();

	/**
	 * @brief Set a plugin specific data
	 *
	 * @param plugin_name The name of the plugin owning the data
	 * @param key The key referencing the data
	 * @param value A void pointer to the data value
	 */
	void SetAttribute(std::string const & plugin_name, std::string const & key,
			VoidPtr_t value);

	/**
	 * @brief Get a plugin specific data
	 *
	 * @param plugin_name The name of the plugin owning the data
	 * @param key The key referencing the data
	 * @return A void pointer to the data value
	 */
	VoidPtr_t GetAttribute(std::string const & plugin_name,
			std::string const & key);

protected:

	/**
	 * @brief Specific plugins data map
	 *
	 * The key is the name of the plugin. Such key references one or more
	 * pairs key-value used to store the specific data.
	 * Inside this pair, the key is the name of the attribute, while the value
	 * is a void pointer to the data. This allows the storage of generic data
	 * types.
	 */
	PlugDataMap_t plugins_data;
};

} // namespace app

} // namespace bbque

#endif // BBQUE_PLUGIN_DATA_H_

