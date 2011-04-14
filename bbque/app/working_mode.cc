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

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"

#include "bbque/app/application.h"
#include "bbque/app/overheads.h"
#include "bbque/app/recipe.h"
#include "bbque/res/resources.h"
#include "bbque/res/resource_accounter.h"

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
	name(_name),
	value(_value) {

	// Application "owner"
	application = _app;

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
	resources.clear();
}


WorkingMode::ExitCode_t WorkingMode::AddResourceUsage(
		std::string const & _res_path, uint64_t _value) {

	br::ResourceAccounter *ra = br::ResourceAccounter::GetInstance();
	br::ResourcePtr_t resource_to_use = ra->GetResource(_res_path);

	// Check if the resource exists
	if (resource_to_use.get() == 0) {
		// Resource requested doesn't exist
		logger->Warn("Resource '%s' not found.", _res_path.c_str());
		return WM_RSRC_NOT_FOUND;
	}

	// Is the usage value correct ? (below the total availability)
	if (_value > resource_to_use->Total()) {
		logger->Warn("Resource '%s' usage value exceeds", _res_path.c_str());
		return WM_RSRC_USAGE_EXCEEDS;
	}

	// Create a new resource usage object and insert it into the map
	resources[_res_path] = UsagePtr_t(new br::ResourceUsage);
	resources[_res_path]->resource = resource_to_use;
	resources[_res_path]->value = _value;
	return WM_SUCCESS;
}


uint64_t WorkingMode::ResourceUsage(std::string const & _res_path) const {

	// Check if the resource exists
	std::map<std::string, UsagePtr_t>::const_iterator it =
		resources.find(_res_path);

	// If the resource has been found return the usage value
	if (it != resources.end())
		return it->second->value;

	// The resource specified is not in use in this working mode
	return 0;
}


WorkingMode::ExitCode_t WorkingMode::AddOverheadInfo(
		std::string const & _dest_awm, double _time) {

	// Check the existance of the destination working mode
	if (application->GetRecipe()->WorkingMode(_dest_awm).get() == NULL) {
		logger->Warn("Working mode %s not found in %s",
		             _dest_awm.c_str(), application->Name().c_str());
		return WM_NOT_FOUND;
	}

	// Have been any AWM switches to '_dest_awm' ?
	std::map<std::string, OverheadPtr_t>::iterator it =
		overheads.find(_dest_awm);

	if (it == overheads.end()) {
		// Create a new overhead object and insert it into the overheads map
		overheads[_dest_awm] = OverheadPtr_t(new TransitionOverheads(_time));
	}
	else {
		// Update existing overhead info
		it->second->IncCount();
		it->second->SetSwitchTime(_time);
	}

	logger->Debug("%s -> %s  overhead [t] :\n", name.c_str(),
			_dest_awm.c_str());
	logger->Debug("\tlast : %.4f", OverheadInfo(_dest_awm)->LastTime());
	logger->Debug("\tmin  : %.4f", OverheadInfo(_dest_awm)->MinTime());
	logger->Debug("\tmax  : %.4f", OverheadInfo(_dest_awm)->MaxTime());
	logger->Debug("\tcount  : %d", OverheadInfo(_dest_awm)->Count());

	return WM_SUCCESS;
}


OverheadPtr_t WorkingMode::OverheadInfo(std::string const & _dest_awm) const {

	OverheadPtr_t over_ptr;
	over_ptr.reset();

	std::map<std::string, OverheadPtr_t>::const_iterator it =
	    overheads.find(_dest_awm);

	// Check if some switch from the current working mode
	// to "_dest_awm" occurred
	if (it != overheads.end())
		over_ptr = it->second;

	// Null pointer
	return over_ptr;
}

} // namespace app

} // namespace bbque

