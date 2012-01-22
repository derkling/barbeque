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
	 * @param value The QoS value read from the recipe
	 */
	explicit WorkingMode(uint8_t id, std::string const & name,
			float value);

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
	inline uint8_t Id() const {
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
	 * @brief Return the value specified in the recipe
	 */
	inline float RecipeValue() const {
		return value.recpv;
	}

	/**
	 * @see WorkingModeStatusIF
	 */
	inline float Value() const {
		return value.normal;
	}

	/**
	 * @brief Set the QoS value specified in the recipe
	 *
	 * The value is viewed as a kind of satisfaction level, from the user
	 * perspective, about the execution of the application.
	 *
	 * @param r_value The QoS value of the working mode
	 */
	inline void SetRecipeValue(float r_value) {
		// Value must be positive
		if (r_value < 0) {
			value.recpv = 0.0;
			return;
		}
		value.recpv = r_value;
	}

	/**
	 * @brief Set the normalized QoS value
	 *
	 * The value is viewed as a kind of satisfaction level, from the user
	 * perspective, about the execution of the application.
	 *
	 * @param n_value The normalized QoS value of the working mode. It must
	 * belong to range [0, 1].
	 */
	inline void SetNormalValue(float n_value) {
		// Normalized value must be in [0, 1]
		if ((n_value < 0.0) || (n_value > 1.0)) {
			logger->Error("SetNormalValue: value not normalized (v = %2.2f)",
					n_value);
			value.normal = 0.0;
			return;
		}
		value.normal = n_value;
	}

	/**
	 * @brief Set a resource usage
	 * @param rsrc_path Resource path
	 * @param amount The usage amount
	 */
	ExitCode_t AddResourceUsage(std::string const & rsrc_path, uint64_t amount);

	/**
	 * @see WorkingModeStatusIF
	 */
	uint64_t ResourceUsageAmount(std::string const & rsrc_path) const;

	/**
	 * @see WorkingModeStatusIF
	 */
	inline UsagesMap_t const & RecipeResourceUsages() const {
		return resources.from_recp;
	}

	/**
	 * @see WorkingModeStatusIF
	 */
	inline size_t NumberOfResourceUsages() const {
		return resources.from_recp.size();
	}

	/**
	 * @see WorkingModeConfIF
	 */
	ExitCode_t AddOverheadInfo(uint8_t dest_awm_id, double time);

	/**
	 * @see WorkingModeStatusIF
	 */
	OverheadPtr_t OverheadInfo(uint8_t dest_awm_id) const;

	/**
	 * @brief Remove switching overhead information
	 * @param dest_awm_id Destination working mode ID
	 */
	inline void RemoveOverheadInfo(uint8_t dest_awm_id) {
		overheads.erase(dest_awm_id);
	}

	/**
	 * @see WorkingModeConfIF
	 */
	ExitCode_t BindResource(std::string const & rsrc_name, ResID_t src_ID,
			ResID_t dst_ID);

	/**
	 * @see WorkingModeStatusIF
	 */
	inline UsagesMapPtr_t GetSchedResourceBinding() const {
		return resources.on_sched;
	}

	/**
	 * @brief Set the resource binding to schedule
	 *
	 * This binds the map of resource usages pointed by "resources.on_sched"
	 * to the WorkingMode. The map will contain ResourceUsage objects
	 * specifying the the amount of resource requested (value) and a list of
	 * system resource descriptors to which bind the request.
	 *
	 * This method is invoked during the scheduling step to track the set of
	 * resources to acquire at the end of the synchronization step.
	 *
	 * @return WM_SUCCESS, or WM_RSRC_MISS_BIND if some bindings are missing
	 */
	ExitCode_t SetSchedResourceBinding();

	/**
	 * @see WorkingModeConfIF
	 */
	inline void ClearSchedResourceBinding() {
		resources.on_sched = UsagesMapPtr_t();
	}

	/**
	 * @brief Get the map of scheduled resource usages
	 *
	 * This method returns the map of the resource usages built through the
	 * mandatory resource binding, in charge of the scheduling policy.
	 *
	 * It is called by the ResourceAccounter to scroll through the list of
	 * resources bound to the working mode assigned.
	 *
	 * @return A shared pointer to a map of ResourceUsage objects
	 */
	inline UsagesMapPtr_t GetResourceBinding() const {
		return resources.to_sync;
	}

	/**
	 * @brief Clear the scheduled resource binding
	 *
	 * The method reverts the effects of ScheduleResourceBinding()
	 */
	inline void ClearResourceBinding() {
		resources.to_sync->clear();
		clusters.curr = clusters.prev;
	}

	/**
	 * @see WorkingModeStatusIF
	 */
	inline ClustersBitSet const & ClusterSet() const {
		return clusters.curr;
	}

	/**
	 * @see WorkingModeStatusIF
	 */
	inline ClustersBitSet const & PrevClusterSet() const {
		return clusters.prev;
	}

	/**
	 * @see WorkingModeStatusIF
	 */
	inline bool ClustersChanged() const {
		return clusters.changed;
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
	uint8_t id;

	/** A descriptive name */
	std::string name;

	/**
	 * @struct WorkingModeValue
	 *
	 * Store information regarding the value of the AWM
	 */
	struct WorkingModeValue {
		/** The QoS value associated to the working mode as specified in the
		 * recipe */
		float recpv;
		/** The normalized QoS value associated to the working mode */
		float normal;
	} value;

	/**
	 * @struct ResourceUsagesInfo
	 *
	 * Store information about the resource requests specified in the recipe
	 * and the bindings built by the scheduling policy
	 */
	struct ResourceUsagesInfo {
		/** The map of resources usages from the recipe  */
		UsagesMap_t from_recp;
		/** The temporary map of resource bindings. This is built by the
		 * BindResource calls */
		UsagesMapPtr_t on_sched;
		/** The map of the resource bindings allocated for the working mode.
		 * This is set by SetSchedResourceBinding() as a commit of the
		 * bindings performed, reasonably by the scheduling policy.	 */
		UsagesMapPtr_t to_sync;
	} resources;

	/** The overheads coming from switching to other working modes */
	OverheadsMap_t overheads;

	/**
	 * @struct ClustersInfo
	 *
	 * The set of clusters used by this working mode. The current and previous
	 * one.
	 */
	struct ClustersInfo {
		/** Save the previous set of clusters bound */
		ClustersBitSet prev;
		/** The current set of clusters bound */
		ClustersBitSet curr;
		/** True if current set differs from previous */
		bool changed;
	} clusters;

	/**
	 * @brief ResourceUsage object referenced by template path
	 *
	 * @param temp_path The resource path (template)
	 * @return The ResourceUsage object from the recipe map.
	 */
	UsagePtr_t ResourceUsageTempRef(std::string const & temp_path) const;

	/**
	 * @brief ResourceUsage object referenced by ID-based path
	 *
	 * @param path The resource path (ID-based)
	 * @return The ResourceUsage object from the binding map. If this is
	 * missing, the recipe map is considered.
	 */
	UsagePtr_t ResourceUsageRef(std::string const & rsrc_path) const;
};

} // namespace app

} // namespace bbque

#endif	// BBQUE_WORKING_MODE_H_

