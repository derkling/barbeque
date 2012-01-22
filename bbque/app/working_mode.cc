/**
 *       @file  working_mode.cc
 *      @brief  Application Working Mode for supporting application execution
 *      in Barbeque RTRM (implementation)
 *
 * The class implement the class for grouping the set of resource requirements
 * (labeled "Application Working Mode") of an application execution profile
 * and a value of Quality of Service.
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

#include "bbque/app/working_mode.h"

#include <string>

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"

#include "bbque/app/application.h"
#include "bbque/app/overheads.h"
#include "bbque/app/recipe.h"
#include "bbque/res/resource_accounter.h"
#include "bbque/utils/utility.h"

namespace br = bbque::res;
namespace bp = bbque::plugins;

namespace bbque { namespace app {


WorkingMode::WorkingMode() {

}


WorkingMode::WorkingMode(uint8_t _id,
		std::string const & _name,
		float _value):
	id(_id),
	name(_name) {

	// Value must be positive
	_value > 0 ?
		value.recpv = _value: value.recpv = 0;

	// Get a logger
	bp::LoggerIF::Configuration conf(AWM_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
}


WorkingMode::~WorkingMode() {
	overheads.clear();
	resources.from_recp.clear();
	resources.to_sync->clear();
}


WorkingMode::ExitCode_t WorkingMode::AddResourceUsage(
		std::string const & rsrc_path,
		uint64_t _value) {
	// Check the total amount of resource
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	uint64_t rsrc_total_qty = ra.Total(rsrc_path);

	// Does the resource exist ?
	if (rsrc_total_qty == 0) {
		logger->Warn("AddResUsage: {%s} not found.", rsrc_path.c_str());
		return WM_RSRC_NOT_FOUND;
	}

	// Is the usage value acceptable ? (below the total availability)
	if (rsrc_total_qty < _value) {
		logger->Warn("AddResUsage: {%s} usage value exceeds total (%d)",
				rsrc_path.c_str(), rsrc_total_qty);
		return WM_RSRC_USAGE_EXCEEDS;
	}

	// Insert a new resource usage object in the map
	UsagePtr_t pusage(UsagePtr_t(new ResourceUsage(_value)));
	resources.from_recp.insert(
			std::pair<std::string, UsagePtr_t>(rsrc_path, pusage));

	logger->Debug("AddResUsage: added {%s}\t[usage: %llu]",
			rsrc_path.c_str(), _value);
	return WM_SUCCESS;
}


uint64_t WorkingMode::ResourceUsageAmount(
		std::string const & rsrc_path) const {
	UsagePtr_t pusage;

	if (IsPathTemplate(rsrc_path))
		pusage = ResourceUsageTempRef(rsrc_path);
	else
		pusage = ResourceUsageRef(rsrc_path);

	// Resource not used by this working mode
	if (!pusage)
		return 0;

	return pusage->GetAmount();
}


UsagePtr_t
WorkingMode::ResourceUsageTempRef(std::string const & temp_path) const {
	UsagesMap_t::const_iterator rsrc_it(resources.from_recp.begin());
	UsagesMap_t::const_iterator it_end(resources.from_recp.end());

	// Iterate over the map of resource usages retrieved from the recipe
	for (; rsrc_it != it_end; ++rsrc_it) {
		// Compare the path with the template of the current resource path
		if (temp_path.compare(PathTemplate(rsrc_it->first)) != 0)
			continue;

		// Return the pointer to the ResourceUsage object
		return rsrc_it->second;
	}

	// Search failed
	return UsagePtr_t();
}


UsagePtr_t
WorkingMode::ResourceUsageRef(std::string const & rsrc_path) const {
	UsagesMap_t::const_iterator rsrc_it;

	// If the resource binding is missing, perform the search in the map of
	// resource usages parsed from the recipe
	if (!resources.to_sync)
		rsrc_it = resources.from_recp.find(rsrc_path);
	else
		rsrc_it = resources.to_sync->find(rsrc_path);

	// Search failed
	if (rsrc_it != resources.from_recp.end())
		return UsagePtr_t();

	// Return the ResourceUsage object
	return rsrc_it->second;
}


WorkingMode::ExitCode_t WorkingMode::AddOverheadInfo(uint8_t _dest_awm_id,
		double _time) {
	// Check the existance of the destination AWM
	assert(owner->GetRecipe().get() != NULL);
	if (!(owner->GetRecipe()->GetWorkingMode(_dest_awm_id))) {
		logger->Warn("AddOvhead: AWM{%d} not found in {%s}",
		             _dest_awm_id, owner->StrId());
		return WM_NOT_FOUND;
	}

	// Update overhead info
	OverheadsMap_t::iterator it(overheads.find(_dest_awm_id));
	if (it == overheads.end()) {
		overheads.insert(std::pair<uint8_t, OverheadPtr_t>(
					_dest_awm_id, OverheadPtr_t(
						new TransitionOverheads(_time))));
	}
	else {
		it->second->IncCount();
		it->second->SetSwitchTime(_time);
	}

	// Debug messages
	logger->Debug("AWM{%d} -> AWM{%d}  overhead [t] :\n", id, _dest_awm_id);
	logger->Debug("\tlast : %.4f", OverheadInfo(_dest_awm_id)->LastTime());
	logger->Debug("\tmin  : %.4f", OverheadInfo(_dest_awm_id)->MinTime());
	logger->Debug("\tmax  : %.4f", OverheadInfo(_dest_awm_id)->MaxTime());
	logger->Debug("\tcount  : %d", OverheadInfo(_dest_awm_id)->Count());

	return WM_SUCCESS;
}


OverheadPtr_t WorkingMode::OverheadInfo(uint8_t _dest_awm_id) const {
	// Overhead descriptor of the destination working mode
	OverheadsMap_t::const_iterator it(overheads.find(_dest_awm_id));
	if (it != overheads.end())
		return it->second;
	// Otherwise... null pointer return
	return OverheadPtr_t();
}


WorkingMode::ExitCode_t WorkingMode::BindResource(
		std::string const & rsrc_name,
		ResID_t src_ID,
		ResID_t dst_ID) {
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	UsagesMap_t::iterator usage_it, it_end;

	// Null name check
	if (rsrc_name.empty()) {
		logger->Error("Binding: Missing resource name");
		return WM_RSRC_ERR_NAME;
	}

	// Allocate a new temporary resource usages map
	UsagesMapPtr_t temp_binds(UsagesMapPtr_t(new UsagesMap_t()));

	// If this is the first binding action, the resource paths to consider
	// must be taken from the recipe resource map. Converserly, if a previous
	// call to this method has been performed, a map of resource usages to
	// schedule has been created. Thus we must continue the binding...
	if (!resources.on_sched) {
		usage_it = resources.from_recp.begin();
		it_end = resources.from_recp.end();
	}
	else {
		usage_it = resources.on_sched->begin();
		it_end = resources.on_sched->end();
	}

	// Proceed with the resource binding...
	for (; usage_it != it_end; ++usage_it) {
		UsagePtr_t & rcp_pusage(usage_it->second);
		std::string const & rcp_path(usage_it->first);

		// Replace resource name+src_ID with resource_name+dst_ID in the
		// resource path
		std::string bind_path(
				ReplaceResourceID(rcp_path, rsrc_name, src_ID, dst_ID));
		logger->Debug("Binding:	'recipe' [%s] => 'bbque' [%s]",
				rcp_path.c_str(), bind_path.c_str());

		// Create a new ResourceUsage object and set the binding list
		UsagePtr_t bind_pusage(new ResourceUsage(rcp_pusage->GetAmount()));
		bind_pusage->SetBindingList(ra.GetResources(bind_path));
		assert(!bind_pusage->EmptyBindingList());

		// Insert the bound resource into the temporary resource usages map
		temp_binds->insert(std::pair<std::string,
					UsagePtr_t>(bind_path, bind_pusage));
	}

	// Update the resource usages map to schedule
	resources.on_sched = temp_binds;

	// Debug messages
	DB(
		usage_it = resources.on_sched->begin();
		it_end = resources.on_sched->end();
		for (; usage_it != it_end; ++usage_it) {
			UsagePtr_t & pusage(usage_it->second);
			std::string const & rcp_path(usage_it->first);

			logger->Debug("Binding: {%s} [amount = %llu #binds = %d]",
					rcp_path.c_str(), pusage->GetAmount(),
					pusage->GetBindingList().size());
		}
	);

	// Are all the resource usages bound ?
	if (resources.from_recp.size() < resources.on_sched->size())
		return WM_RSRC_MISS_BIND;

	return WM_SUCCESS;
}


WorkingMode::ExitCode_t WorkingMode::SetSchedResourceBinding() {
	ClustersBitSet clust_tmp;

	// The binding map must have the same size of resource usages map built
	// from the recipe
	if (!resources.on_sched ||
			(resources.on_sched->size() != resources.from_recp.size()))
		return WM_RSRC_MISS_BIND;

	// Init the iterators for the maps
	UsagesMap_t::iterator bind_it(resources.on_sched->begin());
	UsagesMap_t::iterator end_bind(resources.on_sched->end());
	UsagesMap_t::iterator recp_it(resources.from_recp.begin());
	UsagesMap_t::iterator end_recp(resources.from_recp.end());

	// Check the correctness of the binding
	for(; bind_it != end_bind, recp_it != end_recp; ++recp_it, ++bind_it) {
		std::string const & bind_tmpl(PathTemplate(bind_it->first));
		std::string const & recp_tmpl(PathTemplate(recp_it->first));

		// A mismatch of path template means an error
		if (bind_tmpl.compare(recp_tmpl) != 0) {
			logger->Error("SetBinding: %s resource path mismatch");
			return WM_RSRC_MISS_BIND;
		}

		// Retrieve the bound cluster[s]
		ResID_t cl_id = GetResourceID(bind_tmpl, "cluster");
		if (cl_id == RSRC_ID_NONE)
			continue;

		// Set the bit in the clusters bitset
		logger->Debug("SetBinding: Bound into cluster %d", cl_id);
		clust_tmp.set(cl_id);
	}

	// Update the clusters bitset
	clusters.prev = clusters.curr;
	clusters.curr = clust_tmp;

	// Cluster set changed?
	clusters.changed = clusters.prev != clusters.curr;

	// Set the new binding / resource usages map
	resources.to_sync = resources.on_sched;
	resources.on_sched.reset();

	return WM_SUCCESS;
}

} // namespace app

} // namespace bbque

