/**
 *       @file  application.cc
 *      @brief  Application descriptor implementation
 *
 * This implements the application descriptor.
 * Such descriptor includes static and dynamic information upon application
 * execution. It embeds usual information about name, priority, user, PID
 * (could be different from the one given by OS) plus a reference to the
 * recipe object, the list of enabled working modes and resource constraints.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  06/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/app/application.h"

#include <limits>
#include "bbque/application_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/app/overheads.h"
#include "bbque/app/working_mode.h"
#include "bbque/res/resource_accounter.h"

namespace bbque { namespace app {


Application::Application(std::string const & _name, std::string const & _user,
		uint32_t _pid):
	Object(APPLICATION_NAMESPACE + _name),
	name(_name),
	user(_user),
	pid(_pid) {

	// Scheduling state
	curr_sched.state = next_sched.state = READY;

	// Get a logger
	std::string logger_name(APPLICATION_NAMESPACE + _name);
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger =
		std::unique_ptr<plugins::LoggerIF>
		(ModulesFactory::GetLoggerModule(std::cref(conf)));

	if (logger)
		logger->Info("Starting...");

	enabled_awms.resize(0);
}


Application::~Application() {

	// Release the resources (if any)
	res::ResourceAccounter * ra = bbque::res::ResourceAccounter::GetInstance();
	assert(ra != NULL);
	ra->Release(this);

	// Free the maps
	enabled_awms.clear();
	constraints.clear();
}


void Application::SetPriority(uint16_t _prio) {

	bbque::ApplicationManager * appman =
		bbque::ApplicationManager::GetInstance();

	// If _prio value is greater then the lowest priority
	// (maximum integer value) it is trimmed to the last one.
	priority = std::min(_prio, appman->LowestPriority());
}


AwmStatusPtrList_t const & Application::WorkingModes() {

	// After Recipe loading we have to init the list of working modes enabled.
	// Its should contain all the working modes, if there aren't any
	// constraints asserted
	if ((enabled_awms.size() & constraints.size()) == 0) {

		// This branch is executed just the first time the application is
		// started.
		std::vector<AwmPtr_t> wms = recipe->WorkingModesAll();
		std::vector<AwmPtr_t>::iterator it = wms.begin();
		std::vector<AwmPtr_t>::iterator end = wms.end();

		for ( ; it < end; ++it)
			enabled_awms.push_back(*it);
	}
	return enabled_awms;
}


Application::ExitCode_t Application::SetNextSchedule(
		std::string const & _awm_name, ScheduleFlag_t _state) {

	// Get the working mode pointer
	AwmPtr_t awm = recipe->WorkingMode(_awm_name);

	if (awm.get() == NULL) {
		// AWM name mismatch
		logger->Error("Trying to switch to an unknown working mode");
		return APP_WM_NOT_FOUND;
	}
	// Set next schedule info
	next_sched.awm = awm;
	next_sched.state = _state;
	switch_mark = true;
	return APP_SUCCESS;
}


void Application::SwitchToNextScheduled(double _time) {

	// Check next working mode != current one
	if (curr_sched.awm != next_sched.awm) {

		// Update transition overheads info
		AwmStatusPtr_t curr_awm(curr_sched.awm);
		if (curr_awm) {
			AwmPtr_t _awm(GetRecipe()->WorkingMode(curr_awm->Name()));
			_awm->AddOverheadInfo(next_sched.awm->Name(), _time);
		}
		// Switch to next working mode in scheduling info
		curr_sched.awm = next_sched.awm;

		// Update the current set of usages
		res::ResourceAccounter *res_acc =
			res::ResourceAccounter::GetInstance();

		switch (next_sched.state) {
		case RUNNING:
			// Set new resource usages
			res_acc->SwitchUsage(this);
			break;

		case KILLED:
		case FINISHED:
			// Release resources
			res_acc->Release(this);
		default:
			break;
		}
		logger->Info("Set working mode [%s]", next_sched.awm->Name().c_str());
	}
	// Switch scheduled state
	curr_sched.state = next_sched.state;
	switch_mark = false;
	logger->Info("Set schedule state {%d}",	next_sched.state);
}


Application::ExitCode_t Application::SetConstraint(
		std::string const &	_res_name, Constraint::BoundType _type,
		uint32_t _value) {

	// Lookup the resource by name
	std::map<std::string, ConstrPtr_t>::iterator it_con =
	    constraints.find(_res_name);

	if (it_con == constraints.end()) {
		// Check for resource existance
		res::ResourceAccounter *ra =
		    bbque::res::ResourceAccounter::GetInstance();
		res::ResourcePtr_t rsrc_ptr(ra->GetResource(_res_name));

		if (rsrc_ptr.get() == NULL) {
			// If the resource doesn't exist abort
			return APP_RSRC_NOT_FOUND;
		}
		// Create a constraint object, set its resource reference
		// and insert it into the constraints map
		ConstrPtr_t constr_ptr(new Constraint);
		constr_ptr->resource = rsrc_ptr;
		constraints[_res_name] = constr_ptr;
	}

	// Set the constraint bound value
	switch(_type) {
	case Constraint::LOWER_BOUND:
		constraints[_res_name]->lower = _value;
		break;

	case Constraint::UPPER_BOUND:
		constraints[_res_name]->upper = _value;
		break;
	}
	// Check if there are some awms to disable
	workingModesEnabling(_res_name, _type, _value);

	return APP_SUCCESS;
}


Application::ExitCode_t Application::RemoveConstraint(
		std::string const & _res_name, Constraint::BoundType _type) {

	// Lookup the resource by name
	std::map<std::string, ConstrPtr_t>::iterator it_con =
	    constraints.find(_res_name);

	if (it_con == constraints.end()) {
		logger->Warn("Tried to remove an unknown constraint");
		return APP_CONS_NOT_FOUND;
	}

	// Which kind of constraint ?
	switch (_type) {

	case Constraint::LOWER_BOUND :
		// Remove the lower bound constraint
		it_con->second->lower = 0;
		// Check if there are some awms to re-enable now
		workingModesEnabling(_res_name, _type, it_con->second->lower);
		// Constraint object complete removal ?
		if (it_con->second->upper == std::numeric_limits<uint32_t>::max())
			constraints.erase(it_con);
		break;

	case Constraint::UPPER_BOUND :
		// Remove the upper bound constraint
		it_con->second->upper = std::numeric_limits<uint32_t>::max();
		// Check if there are some awms to re-enable
		workingModesEnabling(_res_name, _type, it_con->second->upper);
		// Constraint object complete removal ?
		if (it_con->second->lower == 0)
			constraints.erase(it_con);
		break;
	}

	return APP_SUCCESS;
}


void Application::workingModesEnabling(std::string const & _res_name,
		Constraint::BoundType _type, uint64_t _value) {

	// Enabled working modes iterators
	std::list<AwmStatusPtr_t>::iterator it_enabl = enabled_awms.begin();
	std::list<AwmStatusPtr_t>::iterator enabl_end = enabled_awms.end();

	// All working modes iterators
	std::vector<AwmPtr_t>::const_iterator it_awm =
		recipe->WorkingModesAll().begin();
	std::vector<AwmPtr_t>::const_iterator awms_end =
		recipe->WorkingModesAll().end();

	uint64_t usage_value;

	// Iterate over all the working modes
	for (; it_awm < awms_end; ++it_awm) {
		usage_value = (*it_awm)->ResourceUsage(_res_name);

		// If a resource usage is below an upper bound constraint or
		// above a lower bound one...
		if (((_type == Constraint::LOWER_BOUND) && (usage_value < _value))  ||
		    ((_type == Constraint::UPPER_BOUND) && (usage_value > _value))) {

			// disable the working mode
			enabled_awms.remove(*it_awm);
			logger->Debug("Disabled : %s", (*it_awm)->Name().c_str());
		} else {
			// The working mode is enabled yet ?
			for( ; it_enabl != enabl_end; ++it_awm) {
				if ((*it_enabl)->Name().compare((*it_awm)->Name()) == 0)
					break;
			}
			if (it_enabl == enabl_end) {
				// No. Enable it
				enabled_awms.push_back(*it_awm);
				logger->Debug("Enabled : %s", (*it_awm)->Name().c_str());
			}
		}
	}
	logger->Debug("%d working modes enabled", enabled_awms.size());
}

} // namespace app

} // namespace bbque

