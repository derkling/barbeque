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

#include "bbque/app/working_mode_conf.h"
#include "bbque/plugins/logger.h"

#define AWM_NAMESPACE "ap.awm"

namespace bbque { namespace app {

/**
 * @class WorkingMode
 * @brief Profile for supporting the application execution.
 *
 * Each Application object should be filled with a list of WorkingMode.
 * A "working mode" is characterized by a set of resource usage request and a
 * "value" which expresses a level of Quality of Service
 */
class WorkingMode: public WorkingModeConfIF {

public:

	/**
	 * @brief Default constructor
	 */
	WorkingMode();

	/**
	 * @brief Constructor with parameters
	 * @param app Application owning the working mode
	 * @param id Working mode ID
	 * @param name Working mode descripting name
	 * @param value The user QoS value of the working mode
	 */
	explicit WorkingMode(uint16_t id, std::string const & name,
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
	 * @see WorkingModeStatusIF
	 */
	inline uint16_t Id() const {
		return id;
	}

	/**
	 * @brief Set the working mode name
	 * @param wm_name The name
	 */
	inline void SetName(std::string const & wm_name) {
		name = wm_name;
	}

	/**
	 * @see WorkingModeStatusIF
	 */
	inline AppPtr_t const & Owner() const {
		return owner;
	}

	/**
	 * @brief Set the application owning the AWM
	 * @param papp Application descriptor pointer
	 */
	inline void SetOwner(AppPtr_t const & papp) {
		owner = papp;
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
	inline UsagesMap_t const * ResourceUsages() const {
		return &rsrc_usages;
	}

	/**
	 * @see WorkingModeStatusIF
	 */
	inline size_t NumberOfResourceUsages() const {
		return rsrc_usages.size();
	}

	/**
	 * @see WorkingModeConfIF
	 */
	ExitCode_t AddOverheadInfo(uint16_t dest_awm_id, double time);

	/**
	 * @see WorkingModeStatusIF
	 */
	OverheadPtr_t OverheadInfo(uint16_t dest_awm_id) const;

	/**
	 * @brief Remove switching overhead information
	 * @param dest_awm_id Destination working mode ID
	 */
	inline void RemoveOverheadInfo(uint16_t dest_awm_id) {
		overheads.erase(dest_awm_id);
	}

	/**
	 * @see WorkingModeConfIF
	 */
	ExitCode_t BindResources(std::string const & rsrc_name,
			ResID_t src_ID, ResID_t dst_ID,
			char * rsrc_path_unb = NULL);

	/**
	 * TODO: This is just a fake  GetBinding
	 */
	inline UsagesMapPtr_t & _GetBinding() {
		return sys_rsrc_usages;
	}

private:

	/** The logger used by the application manager */
	LoggerIF  *logger;

	/**
	 * A pointer to the Application descriptor containing the
	 * current working mode
	 */
	AppPtr_t owner;

	/** A numerical ID  */
	uint16_t id;

	/** A descriptive name */
	std::string name;

	/** The QoS value associated to the working mode */
	uint16_t value;

	/** The set of resources required (usages) */
	UsagesMap_t rsrc_usages;

	UsagesMapPtr_t sys_rsrc_usages;

	/** The overheads coming from switching to other working modes */
	OverheadsMap_t overheads;

};

} // namespace app

} // namespace bbque

#endif	// BBQUE_WORKING_MODE_H_

