/**
 *       @file  working_mode_status.h
 *      @brief  "Read-only" interface for WorkingMode runtime status
 *
 *  This defines the "read-only" interface for WorkingMode runtime status
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

#ifndef BBQUE_WORKING_MODE_STATUS_IF_H_
#define BBQUE_WORKING_MODE_STATUS_IF_H_

#include <bitset>
#include <list>
#include <map>
#include "bbque/app/plugin_data.h"
#include "bbque/res/resource_accounter.h"

#define MAX_NUM_CLUSTERS 6

using bbque::res::ResID_t;
using bbque::res::ResourceUsage;
using bbque::res::UsagePtr_t;
using bbque::res::UsagesMap_t;

namespace bbque { namespace app {

// Forward declarations
class Application;
class TransitionOverheads;

/** Shared pointer to Application  */
typedef std::shared_ptr<Application> AppPtr_t;
/**  Shared pointer to TransitionOverheads */
typedef std::shared_ptr<TransitionOverheads> OverheadPtr_t;
/** Map of OverheadPtr_t. Key: destination working mode name */
typedef std::map<uint8_t, OverheadPtr_t> OverheadsMap_t;
/** Bitset for clusters use monitoring */
typedef std::bitset<MAX_NUM_CLUSTERS> ClustersBitSet;


/**
 * @class WorkingModeStatusIF
 *
 * Read-only interface for the WorkingMode runtime status
 */
class WorkingModeStatusIF: public PluginsData {

public:

	/**
	 * @enum Error codes returned by methods
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
	 * @return ID number
	 */
	virtual uint8_t Id() const = 0;

	/**
	 * @brief Get the application owning the working mode
	 * @return A shared pointer to the application descriptor
	 */
	virtual AppPtr_t const & Owner() const = 0;

	/**
	 * @brief Get the QoS value associated to the working mode
	 */
	virtual float Value() const = 0;

	/**
	 * @brief Get the usage value of a resource
	 * @param res_path Resource path
	 * @return The usage value
	 */
	virtual uint64_t ResourceUsageValue(std::string const & res_path) const = 0;

	/**
	 * @brief Return a map of all the resource usages in the current working
	 * mode.
	 * @return A constant reference to the map of resources
	 */
	virtual UsagesMap_t const & ResourceUsages() const = 0;

	/**
	 * @brief How many resources the working mode uses
	 * @return The number of resource usages
	 */
	virtual size_t NumberOfResourceUsages() const = 0;

	/**
	 * @brief Retrieve overhead information about switching to <tt>awm_name</tt>
	 * working mode
	 * @param dest_awm_id Destination working mode ID
	 * @return A pointer to the TransitionOverheads object
	 */
	virtual OverheadPtr_t OverheadInfo(uint8_t dest_awm_id) const = 0;

	/**
	 * @brief Bind resource usages to system resources descriptors
	 *
	 * Resource paths taken from the recipes use IDs that don't care about the
	 * real system resource IDs registered by Barbeque. Thus a binding must be
	 * solved in order to make the request of resources satisfiables.
	 *
	 * The method takes the resource name we want to bind (i.e. "cluster"),
	 * the ID of the destination system resource, and the source resource ID
	 * (optional). Then it substitutes "cluster+[source ID]" with
	 * "cluster+[dest ID]" and get the descriptor (list of) returned by
	 * ResourceAccounter.
	 *
	 * If the source ID is left blank, the method will bind ALL the
	 * resource path containing "cluster" or "clusterN" to the descriptor
	 * referenced by such path, with the destination resource ID in
	 * the system.
	 *
	 * @param rsrc_name The resource name we want to bind
	 * @param src_ID Recipe resource name source ID
	 * @param dst_ID System resource name destination ID
	 * @param usages_bind A map of resource usages to return with the field
	 * "bind" of ResourceUsage objects filled by binding Resource descriptors
	 * @param rsrc_path_unbound A resource path left to bind
	 * @return An exit code (@see ExitCode_t)
	 *
	 * @note Use RSRC_ID_ANY if you want to bind the resource without care
	 * about its ID.
	 */
	virtual ExitCode_t BindResource(std::string const & rsrc_name,
			ResID_t src_ID,	ResID_t dst_ID,
			UsagesMapPtr_t & usages_bind) = 0;

	/**
	 * @brief Current resource usages bound with the system resources
	 * @return A map of ResourceUsage objects
	 */
	virtual UsagesMapPtr_t GetResourceBinding() const = 0;

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

