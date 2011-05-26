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

#include <limits>

#include "bbque/app/application.h"
#include "bbque/application_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/app/overheads.h"
#include "bbque/app/working_mode.h"
#include "bbque/res/resource_accounter.h"

namespace ba = bbque::app;
namespace br = bbque::res;
namespace bp = bbque::plugins;

namespace bbque { namespace app {

// Compare two working mode values.
// This is used to sort the list of enabled working modes.
bool CompareAWMsByValue(const AwmPtr_t & wm1, const AwmPtr_t & wm2) {
		return wm1->Value() < wm2->Value();
}

Application::Application(std::string const & _name,
		AppPid_t _pid,
		uint8_t _exc_id) :
	Object(APPLICATION_NAMESPACE + _name),
	name(_name),
	pid(_pid),
	exc_id(_exc_id) {

	// Scheduling state
	curr_sched.state = next_sched.state = DISABLED;

	// Get a logger
	std::string logger_name(APPLICATION_NAMESPACE + _name);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = std::unique_ptr<bp::LoggerIF>
		(ModulesFactory::GetLoggerModule(std::cref(conf)));
	assert(logger);

	::snprintf(str_id, 16, "%05d:%6s:%02d",
			Pid(), Name().substr(0,6).c_str(), ExcId());

	logger->Info("Built new EXC [%s]", StrId());

}


Application::~Application() {

	logger->Debug("Destroying EXC [%s]", StrId());
	StopExecution();
}


Application::ExitCode_t Application::StopExecution() {

	logger->Info("Stopping EXC [%s]", StrId());

	// Release the resources
	br::ResourceAccounter * ra = br::ResourceAccounter::GetInstance();
	assert(ra);
	if (!ra) {
		logger->Warn("Stopping EXC [%s] FAILED "
				"(Error: ResourceAccounter unavailable)");
		return APP_ABORT;
	}
	ra->ReleaseUsageSet(this);

	// Release the recipe used
	recipe.reset();

	// Reset scheduling info
	curr_sched.awm.reset();
	next_sched.awm.reset();
	next_sched.state = FINISHED;

	// Releasing AWMs and Constraints...
	enabled_awms.clear();
	constraints.clear();

	return APP_SUCCESS;
}


void Application::SetPriority(AppPrio_t _prio) {

	// Application Manager instance
	bbque::ApplicationManager * appman =
		bbque::ApplicationManager::GetInstance();

	// If _prio value is greater then the lowest priority
	// (maximum integer value) it is trimmed to the last one.
	priority = std::min(_prio, appman->LowestPriority());
}

Application::ExitCode_t Application::Enable() {

	// Not disabled applications could not be marked as READY
	if (curr_sched.state != DISABLED) {
		logger->Crit("Trying to enable already enabled application [%s] "
				"(Error: possible data structure curruption?)",
				StrId());
		assert(curr_sched.state==DISABLED);
		return APP_ABORT;
	}

	next_sched.state = READY;
	logger->Info("EXC [%s] ENABLED", StrId());

	return APP_SUCCESS;

}

Application::ExitCode_t Application::Disable() {
	br::ResourceAccounter * ra(br::ResourceAccounter::GetInstance());

	logger->Debug("Disabling EXC [%s]...", StrId());

	// Not disabled applications could not be marked as READY
	if (curr_sched.state == DISABLED) {
		logger->Crit("Trying to disable already disabled application [%s] "
				"(Error: possible data structure curruption?)",
				StrId());
		assert(curr_sched.state!=DISABLED);
		return APP_ABORT;
	}

	// Release assigned resources
	if (!ra) {
		logger->Warn("Stopping EXC [%s] FAILED "
				"(Error: ResourceAccounter unavailable)");
		assert(ra);
		return APP_ABORT;
	}
	ra->ReleaseUsageSet(this);

	// Reset scheduling info
	curr_sched.awm.reset();
	next_sched.awm.reset();
	next_sched.state = DISABLED;
	logger->Info("EXC [%s] DISABLED", StrId());

	return APP_SUCCESS;

}

void Application::SetRecipe(RecipePtr_t app_recipe) {
	assert(app_recipe.get() != NULL);
	recipe = app_recipe;

	// After Recipe loading we have to init the list of working modes enabled.
	// Its should contain all the working modes, if there aren't any
	// constraints asserted
	if (enabled_awms.empty() && constraints.empty()) {
		// Constraints list is empty. Get all the working modes.
		std::vector<AwmPtr_t> wms = recipe->WorkingModesAll();
		std::vector<AwmPtr_t>::iterator it = wms.begin();
		std::vector<AwmPtr_t>::iterator end = wms.end();

		for (; it < end; ++it)
			enabled_awms.push_back(*it);

		enabled_awms.sort(CompareAWMsByValue);
	}
}


Application::ExitCode_t
Application::SetNextSchedule(AwmPtr_t & n_awm, RViewToken_t vtok) {

	// Get the working mode pointer
	if (!n_awm) {
		logger->Error("Trying to switch to an unknown working mode");
		return APP_WM_NOT_FOUND;
	}

	// Set net working mode and try to acquire the resources
	next_sched.awm = n_awm;
	br::ResourceAccounter *res_acc =
			br::ResourceAccounter::GetInstance();
	if (res_acc->AcquireUsageSet(this, vtok) !=
				br::ResourceAccounter::RA_SUCCESS)
		return APP_WM_REJECTED;

	// Define the transitional scheduling state
	if (curr_sched.awm != n_awm)
		next_sched.state = RECONF;

	// --- manage migration cases here --- //

	return APP_SUCCESS;
}


void Application::UpdateScheduledStatus(double _time) {

	switch (next_sched.state) {
	case MIGREC:
	case MIGRATE:
	case RECONF:
		// Update the reconfiguration overheads
		if (curr_sched.awm) {
			AwmPtr_t _awm = GetRecipe()->WorkingMode(curr_sched.awm->Name());
			_awm->AddOverheadInfo(next_sched.awm->Name(), _time);
		}
	default:
		break;
	}

	// Switch to next working mode in scheduling info
	curr_sched.state = next_sched.state;
	curr_sched.awm = next_sched.awm;
	logger->Info("Scheduled state = {%d}", curr_sched.state);
}


Application::ExitCode_t
Application::SetConstraint(std::string const & _res_name,
		Constraint::BoundType_t _type, uint32_t _value) {

	// Lookup the resource by name
	std::map<std::string, ConstrPtr_t>::iterator it_con =
	    constraints.find(_res_name);

	if (it_con == constraints.end()) {
		// Check for resource existance
		br::ResourceAccounter *ra = br::ResourceAccounter::GetInstance();
		br::ResourcePtr_t rsrc_ptr(ra->GetResource(_res_name));

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


Application::ExitCode_t
Application::RemoveConstraint(std::string const & _res_name,
		Constraint::BoundType_t _type) {

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
		if (it_con->second->upper == std::numeric_limits<uint64_t>::max())
			constraints.erase(it_con);
		break;

	case Constraint::UPPER_BOUND :
		// Remove the upper bound constraint
		it_con->second->upper = std::numeric_limits<uint64_t>::max();
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
		Constraint::BoundType_t _type, uint64_t _value) {

	// Enabled working modes iterators
	AwmPtrList_t::iterator it_enabl = enabled_awms.begin();
	AwmPtrList_t::iterator enabl_end = enabled_awms.end();

	// All working modes iterators
	std::vector<AwmPtr_t>::const_iterator it_awm =
		recipe->WorkingModesAll().begin();
	std::vector<AwmPtr_t>::const_iterator awms_end =
		recipe->WorkingModesAll().end();

	uint64_t usage_value;

	// Iterate over all the working modes
	for (; it_awm < awms_end; ++it_awm) {
		usage_value = (*it_awm)->ResourceUsageValue(_res_name);

		// If a resource usage is below an upper bound constraint or
		// above a lower bound one...
		if (((_type == Constraint::LOWER_BOUND) && (usage_value < _value))  ||
		    ((_type == Constraint::UPPER_BOUND) && (usage_value > _value))) {

			// disable the working mode
			enabled_awms.remove(*it_awm);
			logger->Debug("Disabled : %s", (*it_awm)->Name().c_str());
		}
		else {
			// The working mode is enabled yet ?
			for (; it_enabl != enabl_end; ++it_awm) {
				if ((*it_enabl)->Name().compare((*it_awm)->Name()) == 0)
					break;
			}
			// Check the search result
			if (it_enabl == enabl_end) {
				// No. Enable it
				enabled_awms.push_back(*it_awm);
				logger->Debug("Enabled : %s", (*it_awm)->Name().c_str());
			}
		}
	}

	// Sort by "value"
	enabled_awms.sort(CompareAWMsByValue);
	logger->Debug("%d working modes enabled", enabled_awms.size());
}

} // namespace app

} // namespace bbque

