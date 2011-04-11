/**
 *       @file  plugin_data.h
 *      @brief  Classes for managing plugin specific data attached to
 *      Application and WorkingMode descriptors
 *
 * This define classes for managing plugin specific data. Such data are loaded
 * from the application recipe and can be attached to Application and
 * WorkingMode object.
 * Thus Barbeque plugins can retrieve and update their own data by accessing
 * objects Application and WorkingMode.
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

namespace bbque { namespace app {


/** Map of string type data */
typedef std::map<std::string, std::string> PluginStringDataMap_t;

/** Map of 32-bit integer type data */
typedef std::map<std::string, uint32_t> PluginIntegerDataMap_t;

/** Map of custom (void pointer) data */
typedef std::map<std::string, void *> PluginCustomDataMap_t;


/**
 * @class PluginData
 *
 * This is used for managing pairs key-value used as specific plugin data.
 * We allow the plugin to get and set three type of values: string, integer
 * (32 bit) and custom (void *).
 *
 * The class assorts data from a single plugin.
 */
class PluginData {

public:

	/**
	 *@enum ExitCode_t
	 *
	 * Exit code returned when try to get a value related to a given key
	 */
	typedef enum ExitCode_t {
		/** Success. The value return is valid */
		PDATA_SUCCESS = 0,
		/** Error. Unable to find value with the given key */
		PDATA_ERR_MISS_VALUE

	} ExitCode_t;

	/**
	 * @brief The constructor
	 * @param pl_name Plugin name
	 * @param type Type of plugin
	 * @param req Specify if the plugin is required
	 */
	PluginData(std::string const & pl_name, std::string const & type,
			bool req);

	/**
	 * @brief Get plugin name
	 * @return The name of the plugin
	 */
	inline std::string const & Name() { return plugin_name; }

	/**
	 * @brief Get plugin type
	 * @return The type of plugin
	 */
	inline std::string const & Type() { return type; }

	/**
	 * @brief Check if the plugin is required
	 * @return True for yes, false otherwise
	 */
	inline bool Required() { return required; }

	/**
	 * @brief Insert a data in string format
	 *
	 * Insert a new data as a pair key (string) - value (string)
	 *
	 * @param key Data key
	 * @param value Data value (string)
	 */
	inline void Set(std::string const & key, std::string const & value) {
		str_data[key] = value;
	}

	/**
	 * @brief Insert a data as a 32-bit integer value
	 *
	 * Insert a new data as a pair key (string) - value (integer)
	 *
	 * @param key Data key
	 * @param value Data value (integer)
	 */
	inline void Set(std::string const & key, int32_t value) {
		int_data[key] = value;
	}

	/**
	 * @brief Insert a data as a custom data structure (void pointer)
	 *
	 * Insert a new data as a pair key (string) - data (void *)
	 * @param key Data key
	 * @param value Data value (void *)
	 */
	inline void Set(std::string const & key, void * data) {
		cust_data[key] = data;
	}

	/**
	 * @brief The string value associated to the given key
	 * @param key Data key
	 * @param value Data value returned (string)
	 * @return An exit code about the status of the request (@see ExitCode_t)
	 */
	PluginData::ExitCode_t Get(std::string const & key, std::string & value);

	/**
	 * @brief The 32-bit integer value associated to the given key
	 * @param key Data key
	 * @param value Data value returned (integer)
	 * @return An exit code about the status of the request (@see ExitCode_t)
	 */
	PluginData::ExitCode_t Get(std::string const & key, uint32_t & value);

	/**
	 * @brief The customized data structure associated to the given key
	 * @param key Data key
	 * @param data Void pointer returned (custom data)
	 * @return An exit code about the status of the request (@see ExitCode_t)
	 */
	PluginData::ExitCode_t Get(std::string const & key, void * data);


protected:

	/** Plugin name */
	std::string plugin_name;

	/** Type of plugin */
	std::string type;

	/** True if required */
	bool required;

	/** Set of plugin specific string data */
	PluginStringDataMap_t str_data;

	/** Set of plugin specific integer data */
	PluginIntegerDataMap_t int_data;

	/** Set of plugin specific custom data (void *) */
	PluginCustomDataMap_t cust_data;

};


/** Shared pointer to @ref PluginData */
typedef std::shared_ptr<PluginData> PluginDataPtr_t;


/**
 * @class PluginsDataContainer
 *
 * The class let the managing of specific plugin data. It implements the
 * interface defined in PluginsDataContainerIF. The purpose is to store plugin
 * specific data parsed from the recipe. Such data can be referred to the
 * application or to the working mode. Remind that this are optional
 * information. The suffix "container" means that this objects store data
 * from different plugins.
 * The following class is inherited by Application and WorkingMode.
 * Thus specific plugin can retrieve and store their specific data by
 * accessing objects Application and WorkingMode.
 */
class PluginsDataContainer {

public:

	/** PluginsDataContainer constructor  */
	PluginsDataContainer();

	/**
	 * @brief Add a plugin data set
	 *
	 * Create a PluginData object of a specific plugin in order to
	 * collect data useful for (or required by) that plugin
	 *
	 * @param plugin Plugin name
	 * @param type Type of plugin
	 * @param req Specify if the plugin is required
	 */
	PluginDataPtr_t AddPluginData(std::string const & plugin,
			std::string const & type, bool req);

	/**
	 * @brief Get a plugin data set
	 *
	 * Get the PluginData object of a specific plugin
	 *
	 * @param plugin The name of the plugin
	 * @return A shared pointer to the PluginData object
	 */
	PluginDataPtr_t GetPluginData(std::string const & plugin);

protected:

	/**
	 * @brief Specific plugin data map
	 *
	 * The key is the name of the plugin. The value is a @ref PluginData
	 * object storing data about a single plugin.
	 * Thus each element in the map is a collection of data related to a
	 * specific plugin.
	 */
	std::map<std::string, PluginDataPtr_t> plugins;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_PLUGIN_DATA_H_

