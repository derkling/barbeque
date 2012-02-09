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

#ifndef BBQUE_WORKING_MODE_STATUS_IF_H_
#define BBQUE_WORKING_MODE_STATUS_IF_H_

#include <bitset>
#include <list>
#include <map>

#include "bbque/res/resources.h"
#include "bbque/utils/attributes_container.h"

#define MAX_NUM_CLUSTERS 	8
#define MAX_NUM_BINDINGS 	255

using bbque::res::ResID_t;
using bbque::res::ResourceUsage;
using bbque::res::UsagePtr_t;
using bbque::res::UsagesMap_t;
using bbque::res::UsagesMapPtr_t;
using bbque::utils::AttributesContainer;

namespace bbque { namespace app {


/** Bitset for clusters use monitoring */
typedef std::bitset<MAX_NUM_CLUSTERS> ClustersBitSet;


/**
 * @brief Working mode status interface
 *
 * Read-only interface for the WorkingMode runtime status
 */
class WorkingModeStatusIF: public AttributesContainer {

public:

	/**
	 * @brief Error codes returned by methods
	 */
	enum ExitCode_t {
		/** Success */
		WM_SUCCESS = 0,
		/** Application working mode not found */
		WM_NOT_FOUND,
		/** Resource not found */
		WM_RSRC_NOT_FOUND,
		/** Resource usage request exceeds the total availability */
		WM_RSRC_USAGE_EXCEEDS,
		/** Resource name error */
		WM_RSRC_ERR_NAME,
		/** Missing some resource bindings */
		WM_RSRC_MISS_BIND
	};

	/**
	 * @brief Return the identifying name of the AWM
	 */
	virtual std::string const & Name() const = 0;

	/**
	 * @brief Working Mode ID
	 * @return An integer number
	 */
	virtual uint8_t Id() const = 0;

	/**
	 * @brief Get the application owning the working mode
	 * @return A shared pointer to the application descriptor
	 */
	virtual AppSPtr_t const & Owner() const = 0;

	/**
	 * @brief Get the QoS value associated to the working mode
	 * @return A floating point value
	 */
	virtual float Value() const = 0;

	/**
	 * @brief The amount required of a specific resource
	 * @param res_path Resource path
	 * @return A floating point value
	 */
	virtual uint64_t ResourceUsageAmount(std::string const & res_path)
		const = 0;

	/**
	 * @brief Return a map of all the resource usages in the current working
	 * mode.
	 * @return A constant reference to the map of resources
	 */
	virtual UsagesMap_t const & RecipeResourceUsages() const = 0;

	/**
	 * @brief How many resources the working mode uses
	 * @return The number of resource usages
	 */
	virtual size_t NumberOfResourceUsages() const = 0;

	/**
	 * @brief Current resource usages bound with the system resources
	 *
	 * This is the map of resource usages that should be built by the
	 * scheduling policy. If the method returns a null pointer then the
	 * resources have been set for being allocated yet.
	 *
	 * @param bid A binding identifier. This should be specified if the
	 * scheduling policy aims to handle more than one binding. Currently the
	 * range of acceptable values goes from 0 to 255.
	 *
	 * @return A map of ResourceUsage objects
	 */
	virtual UsagesMapPtr_t GetSchedResourceBinding(uint8_t bid = 0) const = 0;

	/**
	 * @brief Get the bitmap of the clusters currently used.
	 *
	 * Eeach bit set represents a cluster in use. When SetResourceBinding() is
	 * called the set of clusters is properly filled.
	 *
	 * @return A bitset data structure
	 */
	virtual ClustersBitSet const & ClusterSet() const = 0;

	/**
	 * @brief Get the bitmap of the clusters previously used.
	 *
	 * When SetResourceBinding() is called the set of clusters is
	 * saved as "prev".
	 *
	 * @return A bitset data structure
	 */
	virtual ClustersBitSet const & PrevClusterSet() const = 0;

	/**
	 * @brief Check if clusters used are changed
	 *
	 * When a new resource binding is set, if the clusters map has changed keep
	 * track of the change by setting a boolean variable. When the method is
	 * call it returns the value of such variale. This is more efficient than
	 * checking if curr != prev every time.
	 *
	 * @return True if the AWM use a different map of clusters, false otherwise.
	 */
	virtual bool ClustersChanged() const = 0;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_WORKING_MODE_STATUS_IF_H_
