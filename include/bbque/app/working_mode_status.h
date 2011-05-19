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

#include <list>
#include <map>
#include "bbque/app/plugin_data.h"

namespace bbque { namespace res {
	struct ResourceUsage;
}}

namespace br = bbque::res;

namespace bbque { namespace app {

// Forward declarations
class Application;
class TransitionOverheads;

/** Shared pointer to Application  */
typedef std::shared_ptr<Application> AppPtr_t;
/** Shared pointer to ResourceUsage */
typedef std::shared_ptr<br::ResourceUsage> UsagePtr_t;
/** Map of UsagePtr_t. Key: Resource path */
typedef std::map<std::string, UsagePtr_t> UsagesMap_t;
/**  Shared pointer to TransitionOverheads */
typedef std::shared_ptr<TransitionOverheads> OverheadPtr_t;
/** Map of OverheadPtr_t. Key: destination working mode name */
typedef std::map<std::string, OverheadPtr_t> OverheadsMap_t;


/**
 * @class WorkingModeStatusIF
 *
 * Read-only interface for the WorkingMode runtime status
 */
class WorkingModeStatusIF: public PluginsDataContainer {

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
	 * @brief Get the application owning the working mode
	 * @return A shared pointer to the application descriptor
	 */
	virtual AppPtr_t const & OwnerApplication() const = 0;

	/**
	 * @brief Get the QoS value associated to the working mode
	 */
	virtual uint16_t Value() const = 0;

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
	virtual UsagesMap_t const * ResourceUsages() const = 0;

	/**
	 * @brief How many resources the working mode uses
	 * @return The number of resource usages
	 */
	virtual size_t NumberOfResourceUsages() const = 0;

	/**
	 * @brief Retrieve overhead information about switching to <tt>awm_name</tt>
	 * working mode
	 * @param awm_name The destination working mode
	 * @return A pointer to the TransitionOverheads object
	 */
	virtual OverheadPtr_t OverheadInfo(std::string const & awm_name) const = 0;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_WORKING_MODE_STATUS_IF_H_

