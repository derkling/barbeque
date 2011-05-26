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

#include <sstream>
#include <string>

#include "bbque/app/working_mode.h"

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"

#include "bbque/app/application.h"
#include "bbque/app/overheads.h"
#include "bbque/app/recipe.h"
#include "bbque/res/resources.h"

namespace br = bbque::res;
namespace bp = bbque::plugins;

namespace bbque { namespace app {


WorkingMode::WorkingMode():
	bbque::Object(APPLICATION_NAMESPACE WORKING_MODE_NAMESPACE) {
}


WorkingMode::WorkingMode(AppPtr_t _app,	std::string const & _name,
		uint16_t _value):
	bbque::Object(APPLICATION_NAMESPACE + _app->Name() + "." +
			WORKING_MODE_NAMESPACE + _name),
	owner(_app),
	name(_name),
	value(_value) {

	// Get a logger
	std::string logger_name(APPLICATION_NAMESPACE +_app->Name()
			+ "." + WORKING_MODE_NAMESPACE + _name);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger =
		std::unique_ptr<bp::LoggerIF>
		(ModulesFactory::GetLoggerModule(std::cref(conf)));
}


WorkingMode::~WorkingMode() {
	overheads.clear();
	rsrc_usages.clear();
}


WorkingMode::ExitCode_t WorkingMode::AddResourceUsage(
		std::string const & _res_path, uint64_t _value) {

	// Check the total amount of resource
	br::ResourceAccounter &ra = br::ResourceAccounter::GetInstance();
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
	// Create a new resource usage object
	br::UsagePtr_t res_usage = br::UsagePtr_t(new br::ResourceUsage(_value));

	// Insert it into the resource usages map
	rsrc_usages[_res_path] = res_usage;
	return WM_SUCCESS;
}


uint64_t WorkingMode::ResourceUsageValue(
		std::string const & _res_path) const {

	// Check if the resource is really requested
	UsagesMap_t::const_iterator it = rsrc_usages.find(_res_path);

	// If yes return the usage value
	if (it != rsrc_usages.end())
		return it->second->value;

	// No, the resource is not used by this working mode
	return 0;
}


WorkingMode::ExitCode_t WorkingMode::AddOverheadInfo(
		std::string const & _dest_awm, double _time) {

	// Check the existance of the destination working mode
	if (owner->GetRecipe()->WorkingMode(_dest_awm).get() == NULL) {
		logger->Warn("Working mode %s not found in %s",
		             _dest_awm.c_str(), owner->Name().c_str());
		return WM_NOT_FOUND;
	}
	// Have been any AWM switches to '_dest_awm' ?
	std::map<std::string, OverheadPtr_t>::iterator it =
		overheads.find(_dest_awm);
	// If not...
	if (it == overheads.end()) {
		// Create a new overhead object and insert it into the overheads map
		overheads[_dest_awm] = OverheadPtr_t(new TransitionOverheads(_time));
	}
	else {
		// Update existing overhead info
		it->second->IncCount();
		it->second->SetSwitchTime(_time);
	}

	// Some debug messages
	logger->Debug("%s -> %s  overhead [t] :\n", name.c_str(),
			_dest_awm.c_str());
	logger->Debug("\tlast : %.4f", OverheadInfo(_dest_awm)->LastTime());
	logger->Debug("\tmin  : %.4f", OverheadInfo(_dest_awm)->MinTime());
	logger->Debug("\tmax  : %.4f", OverheadInfo(_dest_awm)->MaxTime());
	logger->Debug("\tcount  : %d", OverheadInfo(_dest_awm)->Count());

	return WM_SUCCESS;
}


OverheadPtr_t WorkingMode::OverheadInfo(std::string const & _dest_awm) const {
	// Overhead descriptor
	OverheadPtr_t over_ptr;
	over_ptr.reset();
	// Retrieve the overhead descriptor referring to the given destination
	// working mode
	std::map<std::string, OverheadPtr_t>::const_iterator it =
	    overheads.find(_dest_awm);
	// Any switches from the current working mode to "_dest_awm" in the past ?
	if (it != overheads.end())
		over_ptr = it->second;

	// Otherwise... null pointer return
	return over_ptr;
}


// Append and ID number to the resource name specified, after checking the ID
// validity
inline std::string AppendID(std::string const & _rsrc, ResID_t _id) {

	// Resource name + ID to return
	std::string ret_rsrc = _rsrc;

	// Check ID validity
	if (_id > RSRC_ID_ANY) {
		std::stringstream ss;
		ss << _id;
		ret_rsrc += ss.str();
	}
	return ret_rsrc;
}


WorkingMode::ExitCode_t WorkingMode::BindResources(
		std::string const & rsrc_name, ResID_t src_ID, ResID_t dst_ID,
		const char * rsrc_path_unb) {

	if (rsrc_name.empty())
		return WM_RSRC_ERR_NAME;

	// Number of binding solved
	uint16_t num_of_solved = 0;

	// Resource usages iterators
	UsagesMap_t::iterator usage_it = rsrc_usages.begin();
	UsagesMap_t::iterator it_end = rsrc_usages.end();

	// For each resource usages
	for (; usage_it != it_end; ++usage_it) {

		// Current resource usage path ("recipe view" path).
		// If an ID has been specified, append it to the resource name
		std::string curr_rsrc_path = usage_it->first;
		std::string rsrc_name_match = AppendID(rsrc_name, src_ID);

		// If the resource name is part of the current resource path we must
		// substitute it in the resource path with its binding name
		size_t start_pos = curr_rsrc_path.find(rsrc_name_match);
		if (start_pos != std::string::npos) {

			// Build the binding name
			size_t dot_pos = curr_rsrc_path.find(".", start_pos);
			std::string bind_rsrc_name = AppendID(rsrc_name, dst_ID);

			// Do the replacement into the resource path
			curr_rsrc_path.replace(start_pos, (dot_pos - start_pos),
					bind_rsrc_name);
		}

		// Set the list of resource descriptors matching the binding
		br::ResourceAccounter &ra = br::ResourceAccounter::GetInstance();
		usage_it->second->binds = ra.GetResources(curr_rsrc_path);

		// Update the number of solved bindings
		if (!usage_it->second->binds.empty()) {
			++num_of_solved;
		}
		else {
			// The current binding has not be solved
			if (rsrc_path_unb)
				rsrc_path_unb = usage_it->first.c_str();
		}
	}
	// Are all the resource usages bound ?
	// If the answer is no, the argument "rsrc_path_unbound" will store the
	// path of the last resource not bound.
	if (num_of_solved < rsrc_usages.size())
		return WM_RSRC_MISS_BIND;

	return WM_SUCCESS;
}

} // namespace app

} // namespace bbque

