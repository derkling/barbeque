/**
 *       @file  resource_accounter.cc
 *      @brief  Implementation of the Resource Accounter component
 *
 * This implements the componter for making resource accounting.
 *
 * Each resource of system/platform should be properly registered in the
 * Resource accounter. It keeps track of the information upon availability,
 * total amount and used resources.
 * The information above are updated through proper methods which must be
 * called when an application working mode has triggered.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  04/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/res/resource_accounter.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <locale>
#include <memory>
#include <map>
#include <string>

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/platform_services.h"
#include "bbque/app/application.h"
#include "bbque/app/working_mode.h"

namespace bbque { namespace res {

ResourceAccounter & ResourceAccounter::GetInstance() {
	static ResourceAccounter instance;
	return instance;
}


ResourceAccounter::ResourceAccounter():
	clustering_factor(0) {

	// Get a logger
	std::string logger_name(RESOURCE_ACCOUNTER_NAMESPACE);
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	assert(logger);

	// Init the system resources state view
	sys_usages_view = AppUsagesMapPtr_t(new AppUsagesMap_t);
	sys_view_token = 0;
	usages_per_views[sys_view_token] = sys_usages_view;
	rsrc_per_views[sys_view_token] = ResourceSetPtr_t(new ResourceSet_t);
}


ResourceAccounter::~ResourceAccounter() {
	resources.clear();
	usages_per_views.clear();
	rsrc_per_views.clear();
}


ResourceAccounter::ExitCode_t ResourceAccounter::RegisterResource(
		std::string const & _path,
		std::string const & _units,
		uint64_t _amount) {
	// Check arguments
	if(_path.empty())
		return RA_ERR_MISS_PATH;

	// Insert a new resource in the tree
	ResourcePtr_t res_ptr = resources.insert(_path);
	if (!res_ptr)
		return RA_ERR_MEM;

	// Set the amount of resource considering the units
	res_ptr->SetTotal(ConvertValue(_amount, _units));
	return RA_SUCCESS;
}


uint64_t ResourceAccounter::QueryStatus(ResourcePtrList_t const & rsrc_set,
		QueryOption_t _att,
		RViewToken_t vtok) const {
	// Cumulative value to return
	uint64_t val = 0;

	// For all the descriptors in the list add the quantity of resource in the
	// specified state (available, used, total)
	ResourcePtrList_t::const_iterator res_it(rsrc_set.begin());
	ResourcePtrList_t::const_iterator res_end(rsrc_set.end());
	for (; res_it != res_end; ++res_it) {

		switch(_att) {
		// Resource availability
		case RA_AVAIL:
			val += (*res_it)->Availability(vtok);
			break;
		// Resource used
		case RA_USED:
			val += (*res_it)->Used(vtok);
			break;
		// Resource total
		case RA_TOTAL:
			val += (*res_it)->Total();
			break;
		}
	}
	return val;
}


ResourceAccounter::ExitCode_t ResourceAccounter::AcquireUsageSet(AppPtr_t _app,
		RViewToken_t vtok) {
	// Check to avoid null pointer segmentation fault
	if (!_app)
		return RA_ERR_MISS_APP;

	// Check that the next working mode has been set
	if (!_app->NextAWM())
		return RA_ERR_MISS_USAGES;

	// Get the map of applications resource usages related to the state view
	// referenced by 'vtok'.
	// A missing view implies that the token is not valid.
	AppUsagesMapPtr_t apps_usages;
	if (GetAppUsagesByView(vtok, apps_usages) == RA_ERR_MISS_VIEW)
		return RA_ERR_MISS_VIEW;

	// Each application can hold just one resource usages set
	AppUsagesMap_t::iterator usemap_it(apps_usages->find(_app));
	if (usemap_it != apps_usages->end())
		ReleaseUsageSet(_app, vtok);

	// Set resource usages of the next working mode
	apps_usages->insert(std::pair<AppPtr_t, UsagesMapPtr_t>(
				_app, _app->NextAWM()->ResourceUsages()));
	ExitCode_t ret = IncUsageCounts((*(apps_usages))[_app], _app, vtok);

	// Resource allocation/reservation failed?
	if (ret != RA_SUCCESS)
		apps_usages->erase(_app);

	return ret;
}

ResourceAccounter::ExitCode_t ResourceAccounter::_BookResources(AppPtr_t papp,
		RViewToken_t vtok) {
	(void)papp;
	(void)vtok;
	// NOTE: how you find the proper resource ser? Should it be placed into
	// the method disgnature?!?
	// DO NOT DO HYPOTESYS on other components (e.g. applicatiopn


	// Try booking required resources for the specified application and view
	
	// If successfull: return a success exit code
	
	// Otherwise: release booked resources and return failure exit code

	return RA_SUCCESS;
}

void ResourceAccounter::ReleaseUsageSet(AppPtr_t _app, RViewToken_t vtok) {
	// Check to avoid null pointer seg-fault
	if (!_app)
		return;

	// Get the map of applications resource usages related to the state view
	// referenced by 'vtok'
	AppUsagesMapPtr_t apps_usages;
	if (GetAppUsagesByView(vtok, apps_usages) == RA_ERR_MISS_VIEW)
		return;

	// Get the map of resource usages of the application
	AppUsagesMap_t::iterator usemap_it(apps_usages->find(_app));
	if (usemap_it == apps_usages->end())
		return;

	// Decrement resources counts and remove the usages map
	DecUsageCounts(usemap_it->second, _app, vtok);
	apps_usages->erase(_app);
}


RViewToken_t ResourceAccounter::GetNewView(const char * req_path) {
	// Null-string check
	if (req_path == NULL)
		return -1;

	// Token
	RViewToken_t _token = std::hash<const char *>()(req_path);

	// Allocate a new view for the applications resource usages and the
	// set fo resources allocated
	usages_per_views.insert(std::pair<RViewToken_t, AppUsagesMapPtr_t>(_token,
				AppUsagesMapPtr_t(new AppUsagesMap_t)));
	rsrc_per_views.insert(std::pair<RViewToken_t, ResourceSetPtr_t>(_token,
				ResourceSetPtr_t(new ResourceSet_t)));

	return _token;
}


void ResourceAccounter::PutView(RViewToken_t vtok) {
	// Do nothing if the token references the system state view
	if (vtok == sys_view_token)
		return;

	// Get the resource set using the referenced view
	ResourceViewsMap_t::iterator rviews_it(rsrc_per_views.find(vtok));
	if (rviews_it == rsrc_per_views.end())
		return;

	// For each resource delete the view
	ResourceSet_t::iterator rsrc_set_it(rviews_it->second->begin());
	ResourceSet_t::iterator rsrc_set_end(rviews_it->second->end());
	for (; rsrc_set_it != rsrc_set_end; ++rsrc_set_it) {
		(*rsrc_set_it)->DeleteView(vtok);
	}

	// Remove the map of applications resource usages of this view
	usages_per_views.erase(vtok);
}


RViewToken_t ResourceAccounter::SetAsSystemState(RViewToken_t vtok) {
	// Do nothing if the token references the system state view
	if (vtok == sys_view_token)
		return sys_view_token;

	// Set the system state view pointer to the map of applications resource
	// usages of this view and point to
	AppUsagesViewsMap_t::iterator us_view_it(usages_per_views.find(vtok));
	if (us_view_it == usages_per_views.end())
		return sys_view_token;

	sys_usages_view = us_view_it->second;
	sys_view_token = vtok;

	// Get the resource set using the referenced view
	ResourceViewsMap_t::iterator rviews_it(rsrc_per_views.find(vtok));
	if (rviews_it == rsrc_per_views.end())
		return sys_view_token;

	// For each resource set the view as default
	ResourceSet_t::iterator rsrc_set_it(rviews_it->second->begin());
	ResourceSet_t::iterator rsrc_set_end(rviews_it->second->end());
	for (; rsrc_set_it != rsrc_set_end; ++rsrc_set_it) {
		(*rsrc_set_it)->SetAsDefaultView(vtok);
	}

	return sys_view_token;
}


ResourceAccounter::ExitCode_t ResourceAccounter::GetAppUsagesByView(
		RViewToken_t vtok,
		AppUsagesMapPtr_t & apps_usages) {
	AppUsagesViewsMap_t::iterator view;
	if (vtok != 0) {
		// "Alternate" view case
		view = usages_per_views.find(vtok);
		if (view == usages_per_views.end())
			return RA_ERR_MISS_VIEW;
		apps_usages = view->second;
	}
	else {
		// Default view / System state case
		assert(sys_usages_view);
		apps_usages = sys_usages_view;
	}

	return RA_SUCCESS;
}


inline ResourceAccounter::ExitCode_t ResourceAccounter::IncUsageCounts(
		UsagesMapPtr_t app_usages,
		AppPtr_t _app,
		RViewToken_t vtok) {
	// For each resource usage make a couple of checks
	UsagesMap_t::const_iterator usages_it = app_usages->begin();
	UsagesMap_t::const_iterator usages_end = app_usages->end();
	UsagePtr_t curr_usage;
	for (; usages_it != usages_end; ++usages_it) {

		// Current usage descriptor
		curr_usage = usages_it->second;

		// Is there a resource binding ?
		if (curr_usage->binds.empty())
			return RA_ERR_MISS_BIND;

		// Is the request satisfiable ?
		if (curr_usage->value > QueryStatus(curr_usage->binds, RA_AVAIL, vtok))
			return RA_ERR_USAGE_EXC;
	}

	// Checks passed, iterate again doing the resources reservations
	for (usages_it = app_usages->begin(); usages_it != usages_end;
			++usages_it) {

		// Current usage value to reserve
		curr_usage = usages_it->second;
		uint64_t usage_value = curr_usage->value;

		// Allocate the usage request to the resources binds
		ResourcePtrList_t::iterator it_bind(curr_usage->binds.begin());
		ResourcePtrList_t::iterator end_it(curr_usage->binds.end());
		while ((usage_value > 0) && (it_bind != end_it)) {

			// If the current bind has enough availability, reserve the whole
			// quantity requested here.
			// Otherwise split it in more "sibling" resource binds.
			if (usage_value < (*it_bind)->Availability(vtok))
				usage_value -= (*it_bind)->Acquire(usage_value, _app, vtok);
			else
				usage_value -=
					(*it_bind)->Acquire((*it_bind)->Availability(vtok), _app,
							vtok);

			// Get the resource set using the referenced view and insert the
			// pointer to the resource bind
			ResourceViewsMap_t::iterator rviews_it(rsrc_per_views.find(vtok));
			assert(rviews_it != rsrc_per_views.end());
			rviews_it->second->insert(*it_bind);

			// Next resource bind
			++it_bind;
		}
	}
	return RA_SUCCESS;
}


inline void ResourceAccounter::DecUsageCounts(UsagesMapPtr_t app_usages,
		AppPtr_t _app,
		RViewToken_t vtok) {
	// Release the amount of resource hold by each application
	UsagesMap_t::const_iterator usages_it(app_usages->begin());
	UsagesMap_t::const_iterator usages_end(app_usages->end());
	for (; usages_it != usages_end; ++usages_it) {

		// Resource usage to release / released
		UsagePtr_t curr_usage = usages_it->second;
		uint64_t usage_freed = 0;

		// For each resource bind release the quantity held
		ResourcePtrList_t::iterator it_bind(curr_usage->binds.begin());
		ResourcePtrList_t::iterator end_it(curr_usage->binds.end());
		while (usage_freed < curr_usage->value) {
			assert(it_bind != end_it);
			usage_freed += (*it_bind)->Release(_app, vtok);
			++it_bind;
		}
	}
}

}   // namespace res

}   // namespace bbque

