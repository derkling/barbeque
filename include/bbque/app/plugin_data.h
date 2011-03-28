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

typedef std::map<std::string, std::string> PluginDataMap_t;

/**
 * @class PluginData
 *
 * This is used for managing pairs key-value used as specific plugin data.
 * The class assorts data froma a single plugind.
 */
class PluginData {

public:

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
	 */
	inline std::string const & Name() { return plugin_name; }

	/**
	 * @brief Get plugin type
	 */
	inline std::string const & Type() { return type; }

	/**
	 * @brief Returns true if the plugin is required, false otherwise
	 */
	inline bool Required() { return required; }

	/**
	 * @brief Insert a new data as a pair key-value
	 * @param key Data key
	 * @param value Data value (string)
	 */
	void Set(std::string const & key, std::string const & value) {
		data[key] = value;
	}
	/**
	 * @brief Return the value of the given key
	 * @param key Data key
	 * @return Data value (string)
	 */
	std::string const Get(std::string const & key);

	/**
	 * @brief Return an iterator for the entire map of plugin specific data
	 */
	inline std::map<std::string, std::string>::iterator IteratorBegin() {
		return data.begin();
	}

protected:

	/** Plugin name */
	std::string plugin_name;

	/** Type of plugin */
	std::string type;

	/** True if required */
	bool required;

	/** Set of plugin specific data */
	PluginDataMap_t data;

};


typedef std::shared_ptr<PluginData> PluginDataPtr_t;

/**
 * class PluginDataContainerIF
 *
 * @brief Interface for add and get a plugin specific data container.
 */
class PluginsDataContainerIF {

public:

	/**
	 * @brief Create a PluginData object of a specific plugin in order to
	 * collect data useful for (or required by) that plugin
	 * @param plugin Plugin name
	 * @param type Type of plugin
	 * @param req Specify if the plugin is required
	 */
    virtual PluginDataPtr_t AddPluginData(std::string const & plugin,
			std::string const & type, bool req) = 0;

	/**
	 * @brief Get the PluginData object of a specific plugin
	 * @param plugin The name of the plugin
	 * @return A shared pointer to the PluginData object
	 */
    virtual PluginDataPtr_t GetPluginData(std::string const & plugin) = 0;
};

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

	/** @see PluginsDataContainerIF */
	PluginDataPtr_t AddPluginData(std::string const & plugin,
			std::string const & type, bool req);

	/** @see PluginsDataContainerIF */
    PluginDataPtr_t GetPluginData(std::string const & plugin);

protected:

	/**
	 * @brief Specific plugin data
	 */
	std::map<std::string, PluginDataPtr_t> plugins;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_PLUGIN_DATA_H_

