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

#include "bbque/app/working_mode.h"

#include <string>

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"

#include "bbque/app/application.h"
#include "bbque/res/resource_accounter.h"
#include "bbque/res/resource_utils.h"
#include "bbque/utils/utility.h"

namespace br = bbque::res;
namespace bp = bbque::plugins;

using br::ResourcePathUtils;

namespace bbque { namespace app {


WorkingMode::WorkingMode() {
	resources.on_sched.resize(MAX_NUM_BINDINGS);
}


WorkingMode::WorkingMode(uint8_t _id,
		std::string const & _name,
		float _value):
	id(_id),
	name(_name) {

	// Value must be positive
	_value > 0 ?
		value.recpv = _value: value.recpv = 0;

	// Init the size of the scheduling bindings vector
	resources.on_sched.resize(MAX_NUM_BINDINGS);

	// Get a logger
	bp::LoggerIF::Configuration conf(AWM_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
}


WorkingMode::~WorkingMode() {
	resources.from_recp.clear();
	resources.to_sync->clear();
}


WorkingMode::ExitCode_t WorkingMode::AddResourceUsage(
		std::string const & rsrc_path,
		uint64_t _value) {
	// Template of the resource path
	std::string rsrc_path_tpl(ResourcePathUtils::GetTemplate(rsrc_path));

	// Check the total amount of resource
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	uint64_t rsrc_total_qty = ra.Total(rsrc_path_tpl);

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

	if (ResourcePathUtils::IsTemplate(rsrc_path))
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
		if (temp_path.compare(
					ResourcePathUtils::GetTemplate(rsrc_it->first)) != 0)
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


WorkingMode::ExitCode_t WorkingMode::BindResource(
		std::string const & rsrc_name,
		ResID_t src_ID,
		ResID_t dst_ID,
		uint8_t bid) {
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	UsagesMap_t::iterator usage_it, it_end;

	// Null name check
	if (rsrc_name.empty()) {
		logger->Error("Binding [AWM%d]: Missing resource name", id);
		return WM_RSRC_ERR_NAME;
	}

	// Allocate a new temporary resource usages map
	UsagesMapPtr_t temp_binds(UsagesMapPtr_t(new UsagesMap_t()));

	// If this is the first binding action, the resource paths to consider
	// must be taken from the recipe resource map. Converserly, if a previous
	// call to this method has been performed, a map of resource usages to
	// schedule has been created. Thus we must continue the binding...
	if (!resources.on_sched[bid]) {
		usage_it = resources.from_recp.begin();
		it_end = resources.from_recp.end();
	}
	else {
		usage_it = resources.on_sched[bid]->begin();
		it_end = resources.on_sched[bid]->end();
	}

	// Proceed with the resource binding...
	for (; usage_it != it_end; ++usage_it) {
		UsagePtr_t & rcp_pusage(usage_it->second);
		std::string const & rcp_path(usage_it->first);

		// Replace resource name+src_ID with resource_name+dst_ID in the
		// resource path
		std::string bind_path(
				ResourcePathUtils::ReplaceID(rcp_path, rsrc_name, src_ID,
					dst_ID));
		logger->Debug("Binding [AWM%d]: 'recipe' [%s] => 'bbque' [%s]", id,
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
	resources.on_sched[bid] = temp_binds;

	// Debug messages
	DB(
		usage_it = resources.on_sched[bid]->begin();
		it_end = resources.on_sched[bid]->end();
		for (; usage_it != it_end; ++usage_it) {
			UsagePtr_t & pusage(usage_it->second);
			std::string const & rcp_path(usage_it->first);

			logger->Debug("Binding [AWM%d]: {%s}\t[amount: %llu binds: %d]",
					id, rcp_path.c_str(), pusage->GetAmount(),
					pusage->GetBindingList().size());
		}
		logger->Debug("Binding [AWM%d]: %d resources bound", id,
			resources.on_sched[bid]->size());
	);

	// Are all the resource usages bound ?
	if (resources.from_recp.size() < resources.on_sched[bid]->size())
		return WM_RSRC_MISS_BIND;

	return WM_SUCCESS;
}


WorkingMode::ExitCode_t WorkingMode::SetResourceBinding(uint8_t bid) {
	ClustersBitSet clust_tmp;

	// The binding map must have the same size of resource usages map built
	// from the recipe
	if (!resources.on_sched[bid] ||
			(resources.on_sched[bid]->size() != resources.from_recp.size()))
		return WM_RSRC_MISS_BIND;

	// Init the iterators for the maps
	UsagesMap_t::iterator bind_it(resources.on_sched[bid]->begin());
	UsagesMap_t::iterator end_bind(resources.on_sched[bid]->end());
	UsagesMap_t::iterator recp_it(resources.from_recp.begin());
	UsagesMap_t::iterator end_recp(resources.from_recp.end());

	// Check the correctness of the binding
	for(; bind_it != end_bind, recp_it != end_recp; ++recp_it, ++bind_it) {
		std::string const & bind_tmpl(
				ResourcePathUtils::GetTemplate(bind_it->first));
		std::string const & recp_tmpl(
				ResourcePathUtils::GetTemplate(recp_it->first));

		// A mismatch of path template means an error
		if (bind_tmpl.compare(recp_tmpl) != 0) {
			logger->Error("SetBinding [AWM%d]: %s resource path mismatch %s",
					id, bind_tmpl.c_str(), recp_tmpl.c_str());
			return WM_RSRC_MISS_BIND;
		}

		// Retrieve the bound cluster[s]
		ResID_t cl_id = ResourcePathUtils::GetID(bind_it->first, "cluster");
		if (cl_id == RSRC_ID_NONE)
			continue;

		// Set the bit in the clusters bitset
		logger->Debug("SetBinding [AWM%d]: Bound into cluster %d", id, cl_id);
		clust_tmp.set(cl_id);
	}

	// Update the clusters bitset
	clusters.prev = clusters.curr;
	clusters.curr = clust_tmp;
	logger->Debug("SetBinding [AWM%d]: previous cluster set: %s", id,
			clusters.prev.to_string().c_str());
	logger->Debug("SetBinding [AWM%d]: current cluster set: %s", id,
			clusters.curr.to_string().c_str());

	// Cluster set changed?
	clusters.changed = clusters.prev != clusters.curr;

	// Set the new binding / resource usages map
	resources.to_sync = resources.on_sched[bid];
	resources.on_sched[bid].reset();

	return WM_SUCCESS;
}

} // namespace app

} // namespace bbque

