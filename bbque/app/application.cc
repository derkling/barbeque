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
	name(_name),
	pid(_pid),
	exc_id(_exc_id) {

	// Scheduling state
	curr_sched.state = next_sched.state = DISABLED;

	// Get a logger
	std::string logger_name(APPLICATION_NAMESPACE"." + _name);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
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
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	if (curr_sched.awm)
		ra.ReleaseUsageSet(curr_sched.awm->Owner());

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

	// If _prio value is greater then the lowest priority
	// (maximum integer value) it is trimmed to the last one.
	bbque::ApplicationManager &am(bbque::ApplicationManager::GetInstance());
	priority = std::min(_prio, am.LowestPriority());
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
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());

	logger->Debug("Disabling EXC [%s]...", StrId());

	// Not disabled applications could not be marked as READY
	if (curr_sched.state == DISABLED) {
		logger->Crit("Trying to disable already disabled application [%s] "
				"(Error: possible data structure curruption?)",
				StrId());
		assert(curr_sched.state!=DISABLED);
		return APP_ABORT;
	}

	// Release the resources
	if (curr_sched.awm)
		ra.ReleaseUsageSet(curr_sched.awm->Owner());

	// Reset scheduling info
	curr_sched.awm.reset();
	next_sched.awm.reset();
	next_sched.state = DISABLED;
	logger->Info("EXC [%s] DISABLED", StrId());

	return APP_SUCCESS;
}


void Application::SetRecipe(RecipePtr_t & _recipe, AppPtr_t & papp) {
	// Set the recipe that the application will use
	assert(_recipe.get() != NULL);
	recipe = _recipe;

	// Init information got from the recipe
	InitWorkingModes(papp);
	InitConstraints();
	plugins_data = PlugDataMap_t(recipe->plugins_data);

	// Debug messages
	logger->Info("%d working modes (enabled = %d).", working_modes.size(),
					enabled_awms.size());
	logger->Info("%d constraints in the application.", constraints.size());
	logger->Info("%d plugins specific attributes.", plugins_data.size());
}


void Application::InitWorkingModes(AppPtr_t & papp) {
	// Copy the working modes and set the owner
	AwmMap_t const & wms(recipe->WorkingModesAll());
	AwmMap_t::const_iterator it(wms.begin());
	AwmMap_t::const_iterator end(wms.end());
	for (; it != end; ++it) {
		AwmPtr_t app_awm(new WorkingMode((*(it->second).get())));
		app_awm->SetOwner(papp);
		working_modes.insert(std::pair<uint16_t, AwmPtr_t>(app_awm->Id(),
								app_awm));
		enabled_awms.push_back(app_awm);
	}

	// Sort the enabled list by "value"
	enabled_awms.sort(CompareAWMsByValue);
}


void Application::InitConstraints() {
	// For each static constraint make an assertion
	logger->Warn("%d constraints in the recipe", recipe->ConstraintsAll().size());
	ConstrMap_t::const_iterator cons_it(recipe->ConstraintsAll().begin());
	ConstrMap_t::const_iterator end_cons(recipe->ConstraintsAll().end());
	for (; cons_it != end_cons; ++cons_it) {
		// Lower bound
		if ((cons_it->second)->lower > 0)
				SetConstraint(cons_it->first, Constraint::LOWER_BOUND,
								(cons_it->second)->lower);
		// Upper bound
		if ((cons_it->second)->upper > 0)
				SetConstraint(cons_it->first, Constraint::UPPER_BOUND,
								(cons_it->second)->upper);
	}
}


AwmPtr_t Application::GetWorkingMode(uint16_t wmId) {
	AwmPtrList_t::iterator awm_it(enabled_awms.begin());
	AwmPtrList_t::iterator end_awm(enabled_awms.end());

	while(awm_it != end_awm) {
		if ((*awm_it)->Id() == wmId)
			return (*awm_it);
		++awm_it;
	}

	return AwmPtr_t();
}


Application::ExitCode_t Application::SetNextSchedule(AwmPtr_t const & n_awm,
				RViewToken_t vtok) {
	// Application is blocked, until the AWM validity is verified
	next_sched.state = BLOCKED;

	// Get the working mode pointer
	if (!n_awm) {
		logger->Warn("Working mode rejected: AWM not found");
		return APP_WM_NOT_FOUND;
	}

	// Set next working mode
	next_sched.awm = n_awm;

	// Check resources availability
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
    if (ra.AcquireUsageSet(n_awm->Owner(), vtok)
			!= br::ResourceAccounter::RA_SUCCESS) {
		// Set next working mode to null
		next_sched.awm = AwmPtr_t();
		logger->Warn("Working Mode {%s} rejected by Resource Accounter",
						n_awm->Name().c_str());
		return APP_WM_REJECTED;
	}

	// Define the transitional scheduling state
	if (curr_sched.awm != n_awm)
		next_sched.state = RECONF;

	// --- manage migration cases here --- //

	return APP_SUCCESS;
}


void Application::UpdateScheduledStatus(double _time) {
	// Accordingly to the next scheduling state, update some info
	switch (next_sched.state) {
	case MIGREC:
	case MIGRATE:
	case RECONF:
		// Reconfiguration overheads
		if (curr_sched.awm) {
			AwmPtr_t _awm(GetWorkingMode(curr_sched.awm->Id()));
			_awm->AddOverheadInfo(next_sched.awm->Id(), _time);
		}
	default:
		break;
	}

	// Update current scheduling info
	curr_sched.state = next_sched.state;
	curr_sched.awm = next_sched.awm;
	logger->Info("Scheduled state = {%d}", curr_sched.state);
}


Application::ExitCode_t Application::SetConstraint(
				std::string const & _res_name,
				Constraint::BoundType_t _type,
				uint32_t _value) {
	// Check if the resource exists, and get the descriptor.
	// If doesn't, return.
	ConstrMap_t::iterator it_con(constraints.find(_res_name));
	if (it_con == constraints.end()) {
		br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
		br::ResourcePtr_t rsrc_ptr(ra.GetResource(_res_name));
		if (!rsrc_ptr) {
			logger->Warn("Constraint rejected: unknown resource path");
			return APP_RSRC_NOT_FOUND;
		}

		// Create a constraint object, set its resource reference
		// and insert it into the constraints map
		constraints.insert(std::pair<std::string, ConstrPtr_t>(
								_res_name,
								ConstrPtr_t(new Constraint(rsrc_ptr))));
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
	WorkingModesEnabling(_res_name, _type, _value);

	return APP_SUCCESS;
}


Application::ExitCode_t Application::RemoveConstraint(
				std::string const & _res_name,
				Constraint::BoundType_t _type) {
	// Lookup the constraint by resource pathname
	ConstrMap_t::iterator it_con(constraints.find(_res_name));
	if (it_con == constraints.end()) {
		logger->Warn("Constraint removing failed: unknown resource path");
		return APP_CONS_NOT_FOUND;
	}

	// Reset the constraint and check for AWM to enable
	switch (_type) {
	case Constraint::LOWER_BOUND :
		it_con->second->lower = 0;
		WorkingModesEnabling(_res_name, _type, it_con->second->lower);
		// If there isn't an upper bound value, remove the constraint object
		if (it_con->second->upper == std::numeric_limits<uint64_t>::max())
			constraints.erase(it_con);
		break;

	case Constraint::UPPER_BOUND :
		it_con->second->upper = std::numeric_limits<uint64_t>::max();
		WorkingModesEnabling(_res_name, _type, it_con->second->upper);
		// If there isn't a lower bound value, remove the constraint object
		if (it_con->second->lower == 0)
			constraints.erase(it_con);
		break;
	}

	return APP_SUCCESS;
}


void Application::WorkingModesEnabling(std::string const & _res_name,
				Constraint::BoundType_t _type,
				uint64_t _constr_value) {
	// Iterate over all the working modes to check the resource usage value
	AwmMap_t::const_iterator it_awm(recipe->WorkingModesAll().begin());
	AwmMap_t::const_iterator awms_end(recipe->WorkingModesAll().end());
	for (; it_awm != awms_end; ++it_awm) {

		// If a resource usage is below an upper bound constraint or
		// above a lower bound one disable the working mode, by removing it
		// from the enabled list
		uint64_t usage_value = (it_awm->second)->ResourceUsageValue(_res_name);
		if (((_type == Constraint::LOWER_BOUND) &&
				(usage_value < _constr_value)) ||
						((_type == Constraint::UPPER_BOUND) &&
							(usage_value > _constr_value))) {
			enabled_awms.remove(it_awm->second);
			logger->Debug("Disabled : %s", (it_awm->second)->Name().c_str());
		}
		else {
			// The working mode is enabled yet ?
			AwmPtrList_t::iterator it_enabl(enabled_awms.begin());
			AwmPtrList_t::iterator enabl_end(enabled_awms.end());
			for (; it_enabl != enabl_end; ++it_awm)
				if ((*it_enabl)->Name().compare((it_awm->second)->Name()) == 0)
					break;

			// If no, enable it
			if (it_enabl == enabl_end) {
				enabled_awms.push_back(it_awm->second);
				logger->Debug("Enabled : %s", (it_awm->second)->Name().c_str());
			}
		}
	}

	// Sort by "value"
	enabled_awms.sort(CompareAWMsByValue);
	logger->Debug("%d working modes enabled", enabled_awms.size());
}

} // namespace app

} // namespace bbque

