/**
 *       @file  resources.cc
 *      @brief	Implementation of class Resource
 *
 * A Resource object represents the descriptor of the entity "resource"
 * managed by the Barbeque RTRM. This files provides the implementation of the
 * methods to access, read and update information about a single resource.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  09/08/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/res/resources.h"

#include "bbque/res/resource_accounter.h"


namespace bbque { namespace res {

/*****************************************************************************
 * class Resource
 *****************************************************************************/

Resource::Resource(std::string const & nm):
	name(nm),
	total(1) {
}

Resource::Resource(std::string const & res_path, uint64_t tot):
	total(tot) {

	// Extract the name from the path
	size_t pos = res_path.find_last_of(".");
	if (pos != std::string::npos)
		name = res_path.substr(pos + 1);
	else
		name = res_path;
}

uint64_t Resource::Used(RViewToken_t vtok) {
	// Retrieve the state view
	ResourceStatePtr_t view = GetStateView(vtok);
	if (!view)
		return 0;

	// Return the "used" value
	return view->used;
}

uint64_t Resource::Available(RViewToken_t vtok) {
	// Retrieve the state view
	ResourceStatePtr_t view = GetStateView(vtok);
	if (!view) {
		// If the view is not found, it means that nothing has been allocated.
		// Thus the availability value to return is the total amount of
		// resource
		return total;
	}

	return (total - view->used);
}

uint64_t Resource::Available(AppPtr_t & papp, RViewToken_t vtok) {
	// Retrieve the state view
	ResourceStatePtr_t view = GetStateView(vtok);
	if (!view) {
		// If the view is not found, it means that nothing has been allocated.
		// Thus the availability value to return is the total amount of
		// resource
		return total;
	}

	// Return the amount of available resource plus the amount currently
	// used by the given application
	if (papp)
		return (total - view->used + ApplicationUsage(papp, view->apps));

	// Return the amount of available resource
	return (total - view->used);
}

uint64_t Resource::ApplicationUsage(AppPtr_t & papp, RViewToken_t vtok) {
	// Retrieve the state view
	ResourceStatePtr_t view = GetStateView(vtok);
	if (!view)
		return 0;

	// Call the "low-level" ApplicationUsage()
	return ApplicationUsage(papp, view->apps);
}

Resource::ExitCode_t Resource::UsedBy(AppUid_t & app_uid,
		uint64_t & amount,
		uint8_t idx,
		RViewToken_t vtok) {
	// Get the map of Apps/EXCs using the resource
	AppUseQtyMap_t apps_map;
	size_t mapsize = ApplicationsCount(apps_map, vtok);
	size_t count = 0;
	app_uid = 0;
	amount = 0;

	// Index overflow check
	if (idx >= mapsize)
		return RS_NO_APPS;

	// Search the idx-th App/EXC using the resource
	for (AppUseQtyMap_t::iterator apps_it = apps_map.begin();
			apps_it != apps_map.end(); ++apps_it, ++count) {

		// Skip until the required index has not been reached
		if (count < idx)
			continue;

		// Return the amount of resource used and the App/EXC Uid
		amount = apps_it->second;
		app_uid = apps_it->first;
		return RS_SUCCESS;
	}

	return RS_NO_APPS;
}

uint64_t Resource::Acquire(AppPtr_t const & papp, uint64_t amount,
		RViewToken_t vtok) {
	// Retrieve the state view
	ResourceStatePtr_t view = GetStateView(vtok);
	if (!view) {
		view = ResourceStatePtr_t(new ResourceState());
		state_views[vtok] = view;
	}

	// Try to set the new "used" value
	uint64_t fut_used = view->used + amount;
	if (fut_used > total)
		return 0;

	// Set new used value and application that requested the resource
	view->used = fut_used;
	view->apps[papp->Uid()] = amount;
	return amount;
}

uint64_t Resource::Release(AppPtr_t const & app_ptr, RViewToken_t vtok) {
	// Retrieve the state view
	ResourceStatePtr_t view = GetStateView(vtok);
	if (!view)
		return 0;

	// Lookup the application using the resource
	AppUseQtyMap_t::iterator lkp = view->apps.find(app_ptr->Uid());
	if (lkp == view->apps.end())
		return 0;

	// Decrease the used value and remove the application
	uint64_t used_by_app = lkp->second;
	view->used -= used_by_app;
	view->apps.erase(app_ptr->Uid());

	// Return the amount of resource released
	return used_by_app;
}

void Resource::DeleteView(RViewToken_t vtok) {
	ResourceAccounter &ra(ResourceAccounter::GetInstance());
	// Avoid to delete the default view
	if (vtok == ra.GetSystemView())
		return;
	state_views.erase(vtok);
}

uint16_t Resource::ApplicationsCount(AppUseQtyMap_t & apps_map,
		RViewToken_t vtok) {
	// Retrieve the state view
	ResourceStatePtr_t view = GetStateView(vtok);
	if (!view)
		return 0;

	// Return the size and a reference to the map
	apps_map = view->apps;
	return apps_map.size();
}

uint64_t Resource::ApplicationUsage(AppPtr_t & papp,
		AppUseQtyMap_t & apps_map) {
	// Retrieve the application from the map
	AppUseQtyMap_t::iterator app_using_it(apps_map.find(papp->Uid()));
	if (app_using_it == apps_map.end())
		return 0;

	// Return the amount of resource used
	return app_using_it->second;
}

ResourceStatePtr_t Resource::GetStateView(RViewToken_t vtok) {
	ResourceAccounter &ra(ResourceAccounter::GetInstance());

	// Default view if token = 0
	if (vtok == 0)
		vtok = ra.GetSystemView();

	// Retrieve the view from hash map otherwise
	RSHashMap_t::iterator it = state_views.find(vtok);
	if (it != state_views.end())
		return it->second;

	// State view not found
	return ResourceStatePtr_t();
}

}}
