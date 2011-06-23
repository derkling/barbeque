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
	if(_path.empty()) {
		logger->Fatal("Registering: Invalid resource path");
		return RA_ERR_MISS_PATH;
	}

	// Insert a new resource in the tree
	ResourcePtr_t res_ptr = resources.insert(_path);
	if (!res_ptr) {
		logger->Crit("Registering: Unable to allocate a new resource"
				"descriptor");
		return RA_ERR_MEM;
	}

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

ResourceAccounter::ExitCode_t ResourceAccounter::BookResources(AppPtr_t papp,
		UsagesMapPtr_t const & resource_set,
		RViewToken_t vtok) {
	// Check to avoid null pointer segmentation fault
	if (!papp) {
		logger->Fatal("Booking: Null pointer to the application descriptor");
		return RA_ERR_MISS_APP;
	}
	// Check that the next working mode has been set
	if ((!resource_set) || (resource_set->empty())) {
		logger->Fatal("Booking: Missing a valid resource usages set");
		return RA_ERR_MISS_USAGES;
	}

	// Get the map of resources used by the application (from the state view
	// referenced by 'vtok'). A missing view implies that the token is not
	// valid.
	AppUsagesMapPtr_t apps_usages;
	if (GetAppUsagesByView(vtok, apps_usages) == RA_ERR_MISS_VIEW)
		return RA_ERR_MISS_VIEW;

	// Each application can hold just one resource usages set
	AppUsagesMap_t::iterator usemap_it(apps_usages->find(papp->Uid()));
	if (usemap_it != apps_usages->end())
		return RA_ERR_APP_USAGES;

	// Try booking required resources for the specified application and view
	if (CheckAvailability(resource_set, vtok) == RA_ERR_USAGE_EXC) {
		logger->Debug("Booking: Cannot allocate the resource set");
		return RA_ERR_USAGE_EXC;
	}

	// Increment the booking counts and save the reference to the resource set
	// used by the application
	IncBookingCounts(resource_set, papp, vtok);
	apps_usages->insert(std::pair<AppUid_t, UsagesMapPtr_t>(papp->Uid(),
				resource_set));

	return RA_SUCCESS;
}

void ResourceAccounter::ReleaseResources(AppPtr_t _app, RViewToken_t vtok) {
	// Check to avoid null pointer seg-fault
	if (!_app) {
		logger->Fatal("Release: Null pointer to the application descriptor");
		return;
	}

	// Get the map of applications resource usages related to the state view
	// referenced by 'vtok'
	AppUsagesMapPtr_t apps_usages;
	if (GetAppUsagesByView(vtok, apps_usages) == RA_ERR_MISS_VIEW) {
		logger->Fatal("Release: Resource view unavailable");
		return;
	}

	// Get the map of resource usages of the application
	AppUsagesMap_t::iterator usemap_it(apps_usages->find(_app->Uid()));
	if (usemap_it == apps_usages->end()) {
		logger->Fatal("Release: Application referenced misses a resource set."
				"Possible data corruption occurred.");
		return;
	}

	// Decrement resources counts and remove the usages map
	DecBookingCounts(usemap_it->second, _app, vtok);
	apps_usages->erase(_app->Uid());
}

ResourceAccounter::ExitCode_t ResourceAccounter::CheckAvailability(
		UsagesMapPtr_t const & usages,
		RViewToken_t vtok) const {
	UsagesMap_t::const_iterator usages_it(usages->begin());
	UsagesMap_t::const_iterator usages_end(usages->end());
	for (; usages_it != usages_end; ++usages_it)
		if (Available(usages_it->second, vtok) < usages_it->second->value) {
			logger->Warn("CheckAvail: Exceeding resource request"
					" [usage = %llu]", usages_it->second->value);
			return RA_ERR_USAGE_EXC;
		}
	return RA_SUCCESS;
}

ResourceAccounter::ExitCode_t ResourceAccounter::GetView(const char * req_path,
		RViewToken_t & token) {
	// Null-string check
	if (req_path == NULL) {
		logger->Error("GetView: Missing a valid string");
		return RA_ERR_MISS_PATH;
	}

	// Token
	token = std::hash<const char *>()(req_path);
	logger->Debug("GetView: New resource state view. Token = %d", token);

	// Allocate a new view for the applications resource usages and the
	// set fo resources allocated
	usages_per_views.insert(std::pair<RViewToken_t, AppUsagesMapPtr_t>(token,
				AppUsagesMapPtr_t(new AppUsagesMap_t)));
	rsrc_per_views.insert(std::pair<RViewToken_t, ResourceSetPtr_t>(token,
				ResourceSetPtr_t(new ResourceSet_t)));

	return RA_SUCCESS;
}

void ResourceAccounter::PutView(RViewToken_t vtok) {
	// Do nothing if the token references the system state view
	if (vtok == sys_view_token) {
		logger->Warn("PutView: Cannot release the system resources view");
		return;
	}

	// Get the resource set using the referenced view
	ResourceViewsMap_t::iterator rviews_it(rsrc_per_views.find(vtok));
	if (rviews_it == rsrc_per_views.end()) {
		logger->Error("PutView: Cannot find the resource view referenced by"
				"%llu", vtok);
		return;
	}

	// For each resource delete the view
	ResourceSet_t::iterator rsrc_set_it(rviews_it->second->begin());
	ResourceSet_t::iterator rsrc_set_end(rviews_it->second->end());
	for (; rsrc_set_it != rsrc_set_end; ++rsrc_set_it)
		(*rsrc_set_it)->DeleteView(vtok);

	// Remove the map of applications resource usages of this view
	usages_per_views.erase(vtok);
}

RViewToken_t ResourceAccounter::SetView(RViewToken_t vtok) {
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
	for (; rsrc_set_it != rsrc_set_end; ++rsrc_set_it)
		(*rsrc_set_it)->SetAsDefaultView(vtok);

	return sys_view_token;
}

ResourceAccounter::ExitCode_t ResourceAccounter::GetAppUsagesByView(
		RViewToken_t vtok,
		AppUsagesMapPtr_t & apps_usages) {
	AppUsagesViewsMap_t::iterator view;
	if (vtok != 0) {
		// "Alternate" view case
		view = usages_per_views.find(vtok);
		if (view == usages_per_views.end()) {
			logger->Error("Application usages:"
					"Cannot find the resource view referenced by %llu",
					vtok);
			return RA_ERR_MISS_VIEW;
		}
		apps_usages = view->second;
	}
	else {
		// Default view / System state case
		assert(sys_usages_view);
		apps_usages = sys_usages_view;
	}

	return RA_SUCCESS;
}

void ResourceAccounter::IncBookingCounts(UsagesMapPtr_t const & app_usages,
		AppPtr_t const & papp,
		RViewToken_t vtok) {
	// Get the set of resources referenced in the view
	ResourceViewsMap_t::iterator rviews_it(rsrc_per_views.find(vtok));
	assert(rviews_it != rsrc_per_views.end());

	// Book resources for the application
	UsagesMap_t::const_iterator usages_it(app_usages->begin());
	UsagesMap_t::const_iterator usages_end(app_usages->end());
	for (; usages_it != usages_end;	++usages_it) {
		UsagePtr_t rsrc_usage(usages_it->second);

		// Do booking for this resource (usages_it->second)
		DoResourceBooking(papp, rsrc_usage, vtok, rviews_it->second);
	}
}

void ResourceAccounter::DoResourceBooking(AppPtr_t const & papp,
		UsagePtr_t & rsrc_usage,
		RViewToken_t vtok,
		ResourceSetPtr_t & rsrcs_per_view) {
	// Amount of resource to book
	uint64_t usage_value = rsrc_usage->value;

	// Get the list of resource binds
	ResourcePtrList_t::iterator it_bind(rsrc_usage->binds.begin());
	ResourcePtrList_t::iterator end_it(rsrc_usage->binds.end());
	while ((usage_value > 0) && (it_bind != end_it)) {
		// If the current resource bind has enough availability, reserve the
		// whole quantity requested here. Otherwise split it in more "sibling"
		// resource binds.
		if (usage_value < (*it_bind)->Availability(vtok))
			usage_value -= (*it_bind)->Acquire(usage_value, papp, vtok);
		else
			usage_value -=
				(*it_bind)->Acquire((*it_bind)->Availability(vtok), papp,
						vtok);

		// Add the resource to the set of resources used in the view
		// referenced by 'vtok'
		rsrcs_per_view->insert(*it_bind);

		++it_bind;
	}
}

void ResourceAccounter::DecBookingCounts(UsagesMapPtr_t const & app_usages,
		AppPtr_t const & papp,
		RViewToken_t vtok) {
	// Get the set of resources referenced in the view
	ResourceViewsMap_t::iterator rviews_it(rsrc_per_views.find(vtok));
	assert(rviews_it != rsrc_per_views.end());

	// Release the amount of resource hold by each application
	UsagesMap_t::const_iterator usages_it(app_usages->begin());
	UsagesMap_t::const_iterator usages_end(app_usages->end());
	for (; usages_it != usages_end; ++usages_it) {
		UsagePtr_t rsrc_usage(usages_it->second);

		// Undo booking for this resource (usages_it->second)
		UndoResourceBooking(papp, rsrc_usage, vtok, rviews_it->second);
	}
}

void ResourceAccounter::UndoResourceBooking(AppPtr_t const & papp,
		UsagePtr_t & rsrc_usage,
		RViewToken_t vtok,
		ResourceSetPtr_t & rsrcs_per_view) {
	// Keep track of the amount of resource freed
	uint64_t usage_freed = 0;

	// For each resource bind release the quantity held
	ResourcePtrList_t::iterator it_bind(rsrc_usage->binds.begin());
	ResourcePtrList_t::iterator end_it(rsrc_usage->binds.end());
	while (usage_freed < rsrc_usage->value) {
		assert(it_bind != end_it);
		usage_freed += (*it_bind)->Release(papp, vtok);

		// If there are no more applications using the resource remove it
		// from the set of resources referenced in the view
		if ((*it_bind)->ApplicationsCount() == 0)
			rsrcs_per_view->erase(*it_bind);

		++it_bind;
	}
}


}   // namespace res

}   // namespace bbque

