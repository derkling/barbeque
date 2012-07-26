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

#ifndef BBQUE_WORKING_MODE_H_
#define BBQUE_WORKING_MODE_H_

#include "bbque/app/working_mode_conf.h"
#include "bbque/plugins/logger.h"

#define AWM_NAMESPACE "ap.awm"

namespace bbque { namespace app {


/**
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
	inline AppSPtr_t const & Owner() const {
		return owner;
	}

	/**
	 * @brief Set the application owning the AWM
	 * @param papp Application descriptor pointer
	 */
	inline void SetOwner(AppSPtr_t const & papp) {
		owner = papp;
	}

	/**
	 * @brief Check if the AWM is hidden
	 *
	 * The AWM can be hidden if the current hardware configuration cannot
	 * accomodate the resource requirements included. This happens for example
	 * if the recipe has been profiled on a platform of the same family of the
	 * current one, but with a larger availability of resources, or whenever
	 * the amount of available resources changes at runtime, due to hardware
	 * faults or low-level power/thermal policies. This means that the AWM
	 * must not be taken into account by the scheduling policy.
	 *
	 * @return true if the AWM is hidden, false otherwise.
	 */
	inline bool Hidden() const {
		return hidden;
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
	 * @brief Validate the resource requests according to the current HW
	 * platform configuration/status
	 *
	 * Some HW architectures can be released in several different platform
	 * versions, providing variable amount of resources. Moreover, the
	 * availabilty of resources can vary at runtime due to faults or
	 * low level power/thermal policies. To support such a dynamicity at
	 * runtime it is useful to hide the AWM including a resource requirement
	 * that cannot be satisfied, according to its current total avalability.
	 *
	 * This method performs this check, setting the AWM to "hidden", in case
	 * the condition above is true. Hidden AWMs must not be taken into account
	 * by the scheduling policy
	 */
	ExitCode_t Validate();

	/**
	 * @brief Set the amount of a resource usage request
	 *
	 * This method should be mainly called by the recipe loader.
	 *
	 * @param rsrc_path Resource path
	 * @param amount The usage amount
	 *
	 * @return WM_RSRC_NOT_FOUND if the resource cannot be found in the
	 * system. WM_SUCCESS if the request has been correctly added
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
	ExitCode_t BindResource(std::string const & rsrc_name, ResID_t src_ID,
			ResID_t dst_ID, uint8_t bid = 0);

	/**
	 * @see WorkingModeStatusIF
	 */
	inline UsagesMapPtr_t GetSchedResourceBinding(uint8_t bid = 0) const {
		return resources.on_sched[bid];
	}

	/**
	 * @brief Set the resource binding to schedule
	 *
	 * This binds the map of resource usages pointed by "resources.on_sched"
	 * to the WorkingMode. The map will contain Usage objects
	 * specifying the the amount of resource requested (value) and a list of
	 * system resource descriptors to which bind the request.
	 *
	 * This method is invoked during the scheduling step to track the set of
	 * resources to acquire at the end of the synchronization step.
	 *
	 * @param bid The scheduling binding id to set ready for synchronization
	 * (optional)
	 *
	 * @return WM_SUCCESS, or WM_RSRC_MISS_BIND if some bindings are missing
	 */
	ExitCode_t SetResourceBinding(uint8_t bid = 0);

	/**
	 * @see WorkingModeConfIF
	 */
	inline void ClearSchedResourceBinding() {
		resources.on_sched.clear();
		resources.on_sched.resize(MAX_NUM_BINDINGS);
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
	 * @return A shared pointer to a map of Usage objects
	 */
	inline UsagesMapPtr_t GetResourceBinding() const {
		return resources.to_sync;
	}

	/**
	 * @brief Clear the scheduled resource binding
	 *
	 * The method reverts the effects of SetResourceBinding()
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
	AppSPtr_t owner;

	/** A numerical ID  */
	uint8_t id;

	/** A descriptive name */
	std::string name;

	/**
	 * Whether the AWM includes resource requirements that cannot be
	 * satisfied by the current hardware platform/configuration it can be
	 * flagged as hidden. The idea is to support a dynamic reconfiguration
	 * of the underlying hardware such that some AWMs could be dynamically
	 * taken into account or not at runtime.
	 */
	bool hidden;

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
		std::vector<UsagesMapPtr_t> on_sched;
		/** The map of the resource bindings allocated for the working mode.
		 * This is set by SetResourceBinding() as a commit of the
		 * bindings performed, reasonably by the scheduling policy.	 */
		UsagesMapPtr_t to_sync;
	} resources;

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
	 * @brief Usage object referenced by template path
	 *
	 * @param temp_path The resource path (template)
	 * @return The Usage object from the recipe map.
	 */
	UsagePtr_t ResourceUsageTempRef(std::string const & temp_path) const;

	/**
	 * @brief Usage object referenced by ID-based path
	 *
	 * @param path The resource path (ID-based)
	 * @return The Usage object from the binding map. If this is
	 * missing, the recipe map is considered.
	 */
	UsagePtr_t ResourceUsageRef(std::string const & rsrc_path) const;
};

} // namespace app

} // namespace bbque

#endif	// BBQUE_WORKING_MODE_H_
