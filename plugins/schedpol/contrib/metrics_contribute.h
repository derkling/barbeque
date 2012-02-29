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

#ifndef BBQUE_METRICS_CONTRIBUTE_
#define BBQUE_METRICS_CONTRIBUTE_

#include <cstring>

#include "bbque/configuration_manager.h"
#include "bbque/system_view.h"
#include "bbque/app/application_conf.h"
#include "bbque/app/working_mode.h"
#include "bbque/res/resources.h"
#include "bbque/plugins/logger.h"

#define MCT_CONF_BASE_STR 	"MetricsContribute."
#define MCT_NAME_MAX_LEN 	11

#define for_each_sched_resource_usage(entity, usage_it)             \
	UsagesMapPtr_t const & rsrc_usages(                             \
			entity.pawm->GetSchedResourceBinding(entity.clust_id)); \
	UsagesMap_t::const_iterator end_usage(rsrc_usages->end());      \
	for (usage_it = rsrc_usages->begin();                           \
			usage_it != end_usage; ++usage_it)


using namespace bbque::app;
using namespace bbque::res;

using bbque::app::AwmPtr_t;

namespace bbque { namespace plugins {


/**
 * @brief The scheduling entity to evaluate
 *
 * A scheduling entity is characterized by the Application/EXC to schedule, a
 * Working Mode, and a Cluster ID referencing the resource binding
 */
struct EvalEntity_t {

	/**
	 * @brief Constructor
	 *
	 * @param _papp Application/EXC to schedule
	 * @param _pawm AWM to evaluate
	 * @param _clid Cluster ID for resource binding
	 */
	EvalEntity_t(AppCPtr_t _papp, AwmPtr_t _pawm, uint8_t _clid):
		papp(_papp),
		pawm(_pawm),
		clust_id(_clid) {
		// Log string
		snprintf(str_id, 40, "[%s] {AWM:%d, CL:%d}", papp->StrId(),
				pawm->Id(), clust_id);
	};

	/** Application/EXC to schedule */
	AppCPtr_t papp;
	/** Candidate AWM */
	AwmPtr_t pawm;
	/** Candidate cluster for resource binding */
	ResID_t clust_id;
	/** Identifier string */
	char str_id[40];

	/** Return the identifier string */
	inline const char * StrId() const {
		return str_id;
	}
};


/**
 * @brief Base class for the implementation of scheduling metrics contribute
 *
 * The scheduling policy "YaMS" bases its decision on a modular metrics, made
 * by several contributes. This base class allows the definition of a single
 * metrics contribute. Such contribute must be and index (i.e. € [0, 1])
 * that captures the impact of scheduling the given application, in the
 * working mode specified, having the resources bound into the assigned
 * cluster. The impact must be related to a specific aspect, i.e.
 * reconfiguration costs, migration overheads, power consumption, thermal
 * heatings, and so on...
 */
class MetricsContribute {

public:

	/**
	 * @brief ExitCode_t
	 */
	enum ExitCode_t {
		/** Success */
		MCT_SUCCESS,
		/** No more processing element in a given cluster */
		MCT_RSRC_NO_PE,
		/** A specific resource is not available */
		MCT_RSRC_UNAVL,
		/** Missing a valid resource state view token */
		MCT_ERR_VIEW,
		/** Unexpected error condition */
		MCT_ERROR
	};

	/**
	 * @brief Statistical metrics
	 */
	enum MCTMetrics_t {
		/** Computing time */
		MCT_COMP_TIME,

		MCT_METRICS_COUNT
	};

	/**
	 * @brief Region_t
	 *
	 * According to the current usage level of a resource, we distinguish
	 * among different regions in order to provide a coarse-grained
	 * information about it, that derived class implementing the metrics
	 * contribute computation, could exploit for their evaluations.To be
	 * precise, the idea is to bind a specific function to each region, to
	 * evaluate the impact of a resource requirement. The regions are
	 * distinguished since it is reasonable to think about the aim of penalize
	 * a request the closer it gets the 100% of usage.
	 */
	enum Region_t {
		/** Sub-Saturation Region */
		MCT_RU_SSR,
		/** In-Saturation Region */
		MCT_RU_ISR,
		/** Over-Saturation Region */
		MCT_RU_OSR,

		MCT_RU_COUNT
	};

	/**
	  * @brief Type of resource to manage
	  */
	enum ResourceType_t {
		/** Processing element */
		MCT_RSRC_PE,
		/** Memory */
		MCT_RSRC_MEM,

		MCT_RSRC_COUNT
	};

	/**
	 * @brief Global configuration parameters type
	 */
	enum ConfigParamType_t {
		/** Maximum Saturation Level of processing elements */
		MCT_MSL_PE,
		/** Maximum Saturation Level of memory */
		MCT_MSL_MEM,

		MCT_CPT_COUNT
	};

	/** Global configuration parameters string */
	static char const * ConfigParamsStr[MCT_CPT_COUNT];

	 /** Default values for configuration parameters */
	static uint16_t const ConfigParamsDefault[MCT_CPT_COUNT];

	/**
	 * @brief Levels of resource usage determining the region boundaries
	 */
	struct RegionLevels_t {
		/** Maximum Saturation Level */
		uint64_t saturate;
		/** Current usage level (system resource state)*/
		uint64_t usage;
		/** Amount of resource remaining before reaching the saturation */
		uint64_t free;
		/** Difference between saturation and free resources */
		uint64_t sat_lack;
		/** Total amount of resource */
		uint64_t total;
	};

	/**
	 * @brief Parametes for a generic linear function
	 */
	struct LParams_t {
		/** Scale:     SCALE * x       */
		float scale;
		/** XOffset:   f(x +/- OFFSET) */
		float xoffset;
	};

	/**
	 * @brief Parameters for a generic exponential function
	 */
	struct EParams_t {
		/** Base:      BASE ^ (x)              */
		float base;
		/** XOffset:   base ^ (x +/- OFFSET)   */
		float xoffset;
		/** XScale:    base ^ {SCALE * (x)}    */
		float xscale;
		/** YScale:    SCALE * base ^ (x)      */
		float yscale;
	};

	/**
	 * @brief Parameters for a CLE filter
	 */
	struct CLEParams_t {
		/** Constant */
		float k;
		/** Parameters for the linear function */
		LParams_t lin;
		/** Parameters for the exponential functional */
		EParams_t exp;
	};

	/**
	 * @brief Constructor
	 *
	 * @param name A name identifying the specific contribute
	 * @param cfg_params Global configuration parameters
	 */
	MetricsContribute(const char * name, uint16_t const cfg_params[]);

	/**
	 * @brief Set information for referencing the current state view
	 *
	 * @param _sv Pointer to the SystemView instance
	 * @param _vtok The token of scheduling resource state view
	 */
	inline void SetViewInfo(SystemView * _sv, RViewToken_t _vtok) {
		sv = _sv;
		vtok = _vtok;
	}

	/**
	 * @brief Metrics computation
	 *
	 * Compute the scheduling metrics for the application, taking into account
	 * the working mode specified an thus the bound resource set. The higher
	 * is computd value, the better is the choice of schedule the entity.
	 * Converserly, the lower is the computed the more penalizing would be the
	 * scheduling.
	 *
	 * @param evl_ent The scheduling entity to evaluate (App/AWM/Cluster)
	 * @param ctrib The computed contribute to set
	 *
	 * @return @see ExitCode_t
	 */
	 ExitCode_t Compute(EvalEntity_t const & evl_ent, float & ctrib);


protected:

	 /** Logger */
	 LoggerIF * logger;

	 /** Configuration manager instance */
	 ConfigurationManager & cm;


	 /** Pointer to the SystemView instance */
	 SystemView * sv;

	 /** The token of scheduling resource state view */
	 RViewToken_t vtok;

	 /** Contribute identifier name */
	 char name[MCT_NAME_MAX_LEN];

	 /** Maximum Saturation Levels per resource */
	 float msl_params[MCT_RSRC_COUNT];

	 /** Resource type strings */
	 static char const * rsrc_types_str[MCT_RSRC_COUNT];

	/**
	 * @brief Resource usage region due to a request
	 *
	 * Return the usage region of a resource consequent to the approval of
	 * the given request. This means that the method expresses how the
	 * resource usage level of resource would eventually change. To this
	 * purpose, three region are distinguished:
	 *
	 * 1. SUB-SATURATION: The new resource usage would be included between 0
	 * and the previously scheduled usage level.
	 * 2. IN-SATURATION: The new resource usage would be included between the
	 * previously scheduled usage level and the maximum saturation level
	 * (defined though a configuration parameter).
	 * 3. OVER-SATURATION: The new resource usage would be overpass the
	 * maximum saturation level.
	 *
	 * @param rsrc_path Resource path
	 * @param amount Requested resource usage amount
	 * @param evl_ent Entity to evaluate for scheduling
	 * @param rl Structure embedding info for region usage division
	 *
	 * @return The resulting region of usage of the given resource, whenever
	 * the scheduling satisfies the given request.
	 */
	 Region_t GetUsageRegion(std::string const & rsrc_path, uint64_t amount,
			 EvalEntity_t const & evl_ent, RegionLevels_t & rl);

	 /**
	  * @brief Filter function for resource usage index computation
	  *
	  * Give a resource request, the method returns an index of the goodness
	  * of performing a given resource allocation.
	  *
	  * @brief region The region of the new resource usage
	  * @brief rsrc_usage The new resource usage amount
	  * @brief params The parameters for the internal functions
	  *
	  * @return The index value
	  */
	 float ComputeCLEIndex(Region_t region, float rsrc_usage,
			 CLEParams_t const & params);

	 /***  Static mathematical functions ***/

	 /**
	  * @brief Generic linear function
	  *
	  * @param x The independent variable (assigned)
	  * @param params Parameters for the computation
	  *
	  * @return A floating point value
	  */
	 static float FuncLinear(float x, LParams_t const & params);

	 /**
	  * @brief Generic exponential function
	  *
	  * @param x The independent variable (assigned)
	  * @param params Parameters for the computation
	  *
	  * @return A floating point value
	  */
	 static float FuncExponential(float x, EParams_t const & params);

	 /**
	  * @brief Compute the contribute (override required)
	  *
	  * This must be implemented by the derived class implementing the
	  * specific metrics contribute computation
	  *
	  * @param evl_ent Entity to evaluate for scheduling
	  * @param ctrib The contribute value to set
	  *
	  * @return @see ExitCode_t
	  */
	 virtual ExitCode_t _Compute(EvalEntity_t const & evl_ent, float & ctrib) =
		 0;

};


} // namespace plugins

} // namespace bbque

#endif // BBQUE_METRICS_CONTRIBUTE_
