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
		value = _value: value = 0;

	// Get a logger
	bp::LoggerIF::Configuration conf(AWM_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
}


WorkingMode::~WorkingMode() {
	overheads.clear();
	rsrc_usages.clear();
}


WorkingMode::ExitCode_t WorkingMode::AddResourceUsage(
		std::string const & _res_path,
		uint64_t _value) {
	// Check the total amount of resource
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	uint64_t rsrc_total_qty = ra.Total(_res_path);

	// Does the resource exist ?
	if (rsrc_total_qty == 0) {
		logger->Warn("AddResUsage: {%s} not found.", _res_path.c_str());
		return WM_RSRC_NOT_FOUND;
	}

	// Is the usage value acceptable ? (below the total availability)
	if (rsrc_total_qty < _value) {
		logger->Warn("AddResUsage: {%s} usage value exceeds total (%d)",
				_res_path.c_str(), rsrc_total_qty);
		return WM_RSRC_USAGE_EXCEEDS;
	}

	// Insert a new resource usage object in the map
	br::UsagePtr_t res_usage(br::UsagePtr_t(new br::ResourceUsage(_value)));
	rsrc_usages.insert(std::pair<std::string, br::UsagePtr_t>(
				_res_path, res_usage));

	logger->Debug("AddResUsage: added {%s}\t[usage: %llu]",
			_res_path.c_str(), _value);
	return WM_SUCCESS;
}


uint64_t WorkingMode::ResourceUsageValue(
		std::string const & _res_path) const {
	// Get the resource usage object
	UsagesMap_t::const_iterator it(rsrc_usages.find(_res_path));
	if (it != rsrc_usages.end())
		return it->second->value;
	// Resource not used by this working mode
	return 0;
}


WorkingMode::ExitCode_t WorkingMode::AddOverheadInfo(uint8_t _dest_awm_id,
		double _time) {
	// Check the existance of the destination AWM
	assert(owner->GetRecipe().get() != NULL);
	if (!(owner->GetRecipe()->WorkingMode(_dest_awm_id))) {
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
		ResID_t dst_ID,
		UsagesMapPtr_t & usages_bind) {
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	UsagesMap_t::iterator usage_it(rsrc_usages.begin());
	UsagesMap_t::iterator it_end(rsrc_usages.end());

	// Null name check
	if (rsrc_name.empty()) {
		logger->Error("Binding: Missing resource name");
		return WM_RSRC_ERR_NAME;
	}

	// If the pointer to the UsagesMap_t is null allocate a new map
	if (!usages_bind)
		usages_bind = UsagesMapPtr_t(new UsagesMap_t());

	// Resource usages loaded from the recipe
	for (; usage_it != it_end; ++usage_it) {
		// Replace resource name+src_ID with resource_name+dst_ID in the
		// resource path
		std::string bind_rsrc_path =
				ReplaceResourceID(usage_it->first, rsrc_name, src_ID, dst_ID);

		logger->Debug("Binding: 'recipe' [%s] => 'bbque' [%s]",
				usage_it->first.c_str(), bind_rsrc_path.c_str());

		// Create a new ResourceUsage object, fill "binds" attribute with the
		// list of resource descriptors
		UsagePtr_t bind_rsrc_usage =
			UsagePtr_t(new ResourceUsage(usage_it->second->value));
		bind_rsrc_usage->binds = ra.GetResources(bind_rsrc_path);

		assert(!bind_rsrc_usage->binds.empty());
		logger->Debug("Binding: resources count [%d]",
				bind_rsrc_usage->binds.size());

		// Insert the bound resource into the usages map to return
		usages_bind->insert(std::pair<std::string,
					UsagePtr_t>(bind_rsrc_path, bind_rsrc_usage));
	}

	// Debug messages
	usage_it = usages_bind->begin();
	it_end = usages_bind->end();
	for (; usage_it != it_end; ++usage_it) {
		logger->Debug("Binding: {%s} [value = %llu #binds = %d]",
				usage_it->first.c_str(),
				usage_it->second->value,
				usage_it->second->binds.size());
	}

	// Are all the resource usages bound ?
	if (rsrc_usages.size() < usages_bind->size())
		return WM_RSRC_MISS_BIND;

	return WM_SUCCESS;
}


WorkingMode::ExitCode_t WorkingMode::SetResourceBinding(
		UsagesMapPtr_t & usages_bind) {
	// If the bind map has a size different from the resource usages one
	// return
	if (usages_bind->size() != rsrc_usages.size())
		return WM_RSRC_MISS_BIND;

	UsagesMap_t::iterator bind_it(usages_bind->begin());
	UsagesMap_t::iterator end_bind(usages_bind->end());
	UsagesMap_t::iterator rsrc_it(rsrc_usages.begin());
	UsagesMap_t::iterator end_rsrc(rsrc_usages.end());

	// If there's a path template mismatch returns
	while ((bind_it != end_bind) && (rsrc_it != end_rsrc)) {
		if (PathTemplate(bind_it->first).compare(
					PathTemplate(rsrc_it->first)) != 0) {
			logger->Error("SetBinding: %s resource path mismatch");
			return WM_RSRC_MISS_BIND;
		}

		++rsrc_it;
		++bind_it;
	}

	// Update the clusters bitset
	cluster_set.prev = cluster_set.curr;
	cluster_set.curr.reset();

	bind_it = usages_bind->begin();
	end_bind = usages_bind->end();
	for (; bind_it != end_bind; ++bind_it) {
		// If this is a clustered resource mark the cluster bit in the set
		ResID_t cl_id = GetResourceID(bind_it->first, "cluster");
		if (cl_id != RSRC_ID_NONE) {
			logger->Debug("SetBinding: Bound into cluster %d", cl_id);
			cluster_set.curr.set(cl_id);
		}
	}

	// Cluster set changed ?
	cluster_set.changed = cluster_set.prev != cluster_set.curr;

	// Set the bind map
	sys_rsrc_usages = usages_bind;
	return WM_SUCCESS;
}

} // namespace app

} // namespace bbque

