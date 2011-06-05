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

#include <sstream>
#include <string>

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"

#include "bbque/app/application.h"
#include "bbque/app/overheads.h"
#include "bbque/app/recipe.h"
#include "bbque/res/resources.h"

namespace br = bbque::res;
namespace bp = bbque::plugins;

namespace bbque { namespace app {


WorkingMode::WorkingMode() {

}


WorkingMode::WorkingMode(uint16_t _id,
		std::string const & _name,
		uint16_t _value):
	id(_id),
	name(_name),
	value(_value) {

	// Get a logger
	std::string logger_name(AWM_NAMESPACE ".");
	bp::LoggerIF::Configuration conf(logger_name.c_str());
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
		logger->Warn("Resource '%s' not found.", _res_path.c_str());
		return WM_RSRC_NOT_FOUND;
	}

	// Is the usage value acceptable ? (below the total availability)
	if (rsrc_total_qty < _value) {
		logger->Warn("Resource '%s' usage value exceeds \n"
				"(total = %d)", _res_path.c_str(), rsrc_total_qty);
		return WM_RSRC_USAGE_EXCEEDS;
	}

	// Insert a new resource usage object in the map
	br::UsagePtr_t res_usage(br::UsagePtr_t(new br::ResourceUsage(_value)));
	rsrc_usages.insert(std::pair<std::string, br::UsagePtr_t>(
				_res_path, res_usage));

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


WorkingMode::ExitCode_t WorkingMode::AddOverheadInfo(uint16_t _dest_awm_id,
		double _time) {
	// Check the existance of the destination AWM
	assert(owner->GetRecipe().get() != NULL);
	if (!(owner->GetRecipe()->WorkingMode(_dest_awm_id))) {
		logger->Warn("Working mode ID=%d not found in %s",
		             _dest_awm_id, owner->Name().c_str());
		return WM_NOT_FOUND;
	}

	// Update overhead info
	OverheadsMap_t::iterator it(overheads.find(_dest_awm_id));
	if (it == overheads.end()) {
		overheads.insert(std::pair<uint16_t, OverheadPtr_t>(
					_dest_awm_id, OverheadPtr_t(new TransitionOverheads(_time))));
	}
	else {
		it->second->IncCount();
		it->second->SetSwitchTime(_time);
	}

	// Debug messages
	logger->Debug("AWM %d -> AWM%d  overhead [t] :\n", id, _dest_awm_id);
	logger->Debug("\tlast : %.4f", OverheadInfo(_dest_awm_id)->LastTime());
	logger->Debug("\tmin  : %.4f", OverheadInfo(_dest_awm_id)->MinTime());
	logger->Debug("\tmax  : %.4f", OverheadInfo(_dest_awm_id)->MaxTime());
	logger->Debug("\tcount  : %d", OverheadInfo(_dest_awm_id)->Count());

	return WM_SUCCESS;
}


OverheadPtr_t WorkingMode::OverheadInfo(uint16_t _dest_awm_id) const {
	// Overhead descriptor of the destination working mode
	OverheadsMap_t::const_iterator it(overheads.find(_dest_awm_id));
	if (it != overheads.end())
		return it->second;
	// Otherwise... null pointer return
	return OverheadPtr_t();
}


// Append and ID number to the resource name specified, after checking the ID
// validity
inline std::string AppendID(std::string const & _rsrc, ResID_t _id) {
	// Resource name + ID to return
	std::string ret_rsrc(_rsrc);

	// Check ID validity
	if (_id > RSRC_ID_ANY) {
		std::stringstream ss;
		ss << _id;
		ret_rsrc += ss.str();
	}
	return ret_rsrc;
}


WorkingMode::ExitCode_t WorkingMode::BindResources(
		std::string const & rsrc_name,
		ResID_t src_ID,
		ResID_t dst_ID,
		char * rsrc_path_unb) {
	// Null path check
	if (rsrc_name.empty())
		return WM_RSRC_ERR_NAME;

	// Number of binding solved
	uint16_t num_of_solved = 0;

	// Resource usages
	UsagesMap_t::iterator usage_it(rsrc_usages.begin());
	UsagesMap_t::iterator it_end(rsrc_usages.end());
	for (; usage_it != it_end; ++usage_it) {

		// Current resource usage path ("recipe view" path).
		std::string curr_rsrc_path(usage_it->first);
		std::string rsrc_name_match(AppendID(rsrc_name, src_ID));

		// Is the resource name to bind part of the current resource path ?
		size_t start_pos = curr_rsrc_path.find(rsrc_name_match);
		if (start_pos != std::string::npos) {

			// Build the new binding name and replace into the path
			size_t dot_pos = curr_rsrc_path.find(".", start_pos);
			std::string bind_rsrc_name(AppendID(rsrc_name, dst_ID));
			curr_rsrc_path.replace(start_pos, (dot_pos - start_pos),
					bind_rsrc_name);
		}

		// Set the list of resource descriptors matching the binding
		br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
		usage_it->second->binds = ra.GetResources(curr_rsrc_path);

		// If the current binding has not be solved, save the path of the last
		// unbound resource
		if (!usage_it->second->binds.empty()) {
			if (rsrc_path_unb)
				strncpy(rsrc_path_unb, usage_it->first.c_str(),
						(usage_it->first).size());
			continue;
		}

		// Update the number of solved bindings
		++num_of_solved;
	}

	// Are all the resource usages bound ?
	if (num_of_solved < rsrc_usages.size())
		return WM_RSRC_MISS_BIND;

	return WM_SUCCESS;
}

} // namespace app

} // namespace bbque

