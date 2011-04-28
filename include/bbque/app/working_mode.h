/**
 *       @file  working_mode.h
 *      @brief  Application Working Mode for supporting application execution
 *      in Barbeque RTRM
 *
 * The class defines the set of resource requirements of an application
 * execution profile (labeled "Application Working Mode") and a value of
 * Quality of Service.
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

#ifndef BBQUE_WORKING_MODE_H_
#define BBQUE_WORKING_MODE_H_

#include "bbque/object.h"
#include "bbque/app/working_mode_conf.h"

#define WORKING_MODE_NAMESPACE "awms."

namespace bbque { namespace app {

/**
 * @class WorkingMode
 * @brief Profile for supporting the application execution.
 *
 * Each Application object should be filled with a list of WorkingMode.
 * A "working mode" is characterized by a set of resource usage request and a
 * "value" which expresses a level of Quality of Service
 */
class WorkingMode: public Object, public WorkingModeConfIF {

public:

	/**
	 * @brief Default constructor
	 */
	WorkingMode();

	/**
	 * @brief Constructor with parameters
	 * @param app A pointer to the application which to refer
	 * @param name The working mode name
	 * @param value The user QoS value of the working mode
	 */
	explicit WorkingMode(AppPtr_t app, std::string const & name,
			uint16_t value);

	/**
	 * @brief Default destructor
	 */
	~WorkingMode();

	/**
	 * @see WorkingModeStatusIF
	 */
	inline std::string const & Name() const {
		return name;
	}

	/**
	 * @brief Set the working mode name
	 * @param wm_name The name
	 */
	inline void SetName(std::string const & wm_name) {
		name = wm_name;
	}

	/**
	 * @brief see WorkingModeStatusIF
	 */
	inline AppPtr_t const & OwnerApplication() const {
		return owner;
	}

	/**
	 * @see WorkingModeStatusIF
	 */
	inline uint16_t Value() const {
		return value;
	}

	/**
	 * @see WorkingModeConfIF
	 */
	inline void SetValue(uint16_t qos_value) {
		value = qos_value;
	}

	/**
	 * @brief Set a resource usage
	 * @param res_path Resource path
	 * @param value The usage value
	 */
	ExitCode_t AddResourceUsage(std::string const & res_path, uint64_t value);

	/**
	 * @see WorkingModeStatusIF
	 */
	uint64_t ResourceUsageValue(std::string const & res_path) const;

	/**
	 * @see WorkingModeStatusIF
	 */
	inline UsagesMap_t const & ResourceUsages() const {
		return resources;
	}

	/**
	 * @see WorkingModeStatusIF
	 */
	OverheadPtr_t OverheadInfo(std::string const & dest_awm) const;

	/**
	 * @see WorkingModeConfIF
	 */
	ExitCode_t AddOverheadInfo(std::string const & dest_awm, double time);

	/**
	 * @brief Remove overhead informations about switching to 'dest_wm'
	 * working mode
	 * @param dest_awm The destination working mode
	 */
	inline void RemoveOverheadInfo(std::string const & dest_awm) {
		overheads.erase(dest_awm);
	}

private:

	/**
	 * A pointer to the Application descriptor containing the
	 * current working mode
	 */
	AppPtr_t owner;

	/** The identifier name */
	std::string name;

	/** The QoS value associated to the working mode */
	uint16_t value;

	/** The set of resources required (usages) */
	UsagesMap_t resources;

	/** The overheads coming from switching to other working modes */
	OverheadsMap_t overheads;

};

} // namespace app

} // namespace bbque

#endif	// BBQUE_WORKING_MODE_H_

