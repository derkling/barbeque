/**
 *       @file  resource_accounter.cc
 *      @brief  Implementation of the Resource Accounter component
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
#include <memory>
#include <map>
#include <string>

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/platform_services.h"

#include "bbque/app/application.h"
#include "bbque/app/working_mode.h"


namespace bbque { namespace res {


ResourceAccounter *ResourceAccounter::GetInstance() {
	static ResourceAccounter *instance = NULL;

	if (instance == NULL)
		instance = new ResourceAccounter();
	return instance;
}


ResourceAccounter::ResourceAccounter():
	bbque::Object(RESOURCE_ACCOUNTER_NAMESPACE) {

	// Get a logger
	std::string logger_name(RESOURCE_ACCOUNTER_NAMESPACE);
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger =
		std::unique_ptr<plugins::LoggerIF>
		(ModulesFactory::GetLoggerModule(std::cref(conf)));

	if (logger)
		logger->Debug("ResourceAccounter instance.");
}


ResourceAccounter::~ResourceAccounter() {
	resources.clear();
	usages.clear();
}


void ResourceAccounter::RegisterResource(std::string const & _path,
		std::string	const & _type, std::string const & _units,
		uint32_t _amount) {

	assert(!_path.empty());
	assert(!_type.empty());

	// Create a new resource path
	ResourcePtr_t res_ptr;
	res_ptr = resources.insert(_path);

	if (res_ptr.get() != NULL) {

		// Set the amount of resource on the units base
		switch(toupper(_units.at(0))) {
		case 'K':
			res_ptr->SetTotal(_amount * pow(2, 10));
			break;
		case 'M':
			res_ptr->SetTotal(_amount * pow(2, 20));
			break;
		case 'G':
			res_ptr->SetTotal(_amount * pow(2, 30));
			break;
		case 'T':
			res_ptr->SetTotal(_amount * pow(2, 40));
			break;
		default:
			res_ptr->SetTotal(_amount);
		}
		// Resource type
		res_ptr->SetType(_type);
	}
}


uint64_t ResourceAccounter::stateInformation(std::string const & _path,
		AttributeSelector_t _att) const {

	// Lookup the resource descriptor by path
	ResourcePtr_t res_desc = resources.find(_path);
	if (res_desc.get() != NULL) {

		switch(_att) {
			// Resource availability
		case RA_AVAILAB:
			return res_desc->Availability();
			// Resource used
		case RA_USED:
			return res_desc->Used();
			// Resource total
		case RA_TOTAL:
			return res_desc->Total();
		}
	}
	return 0;
}


void ResourceAccounter::changeUsages(ba::Application const * _app,
		UsageAction_t _sel) {

	// Map of resource usages of the application
	UsagesMap_t const * app_usages;

	switch (_sel) {
	case RA_SWITCH:
		// Check the next working mode is set
		if(_app->NextAWM().get() == NULL)
			return;
		// Release current resources
		changeUsages(_app, RA_RELEASE);

		// Set resources usages from the next working mode
		usages[_app->Name()] = &(_app->NextAWM()->ResourceUsages());
		app_usages = usages[_app->Name()];
		break;

	case RA_RELEASE:
		// Get the map of resource usages of the application
		std::map<std::string, UsagesMap_t const *>::iterator usemap_it;
		usemap_it = usages.find(_app->Name());

		// If there isn't any map return
		if (usemap_it == usages.end())
			return;
		// Retrieve the current application map of usages
		app_usages = usemap_it->second;
		if (app_usages->size() == 0)
			return;
	}

	// ResourceUsages iterators
	std::map<std::string, UsagePtr_t>::const_iterator usages_it =
		app_usages->begin();
	std::map<std::string, UsagePtr_t>::const_iterator usages_end =
		app_usages->end();

	// For each resource in the usages map...
	for (; usages_it != usages_end; ++usages_it) {

		// Lookup the resource descriptor
		ResourcePtr_t res_desc(usages_it->second->resource);
		if (res_desc.get() == NULL)
			continue;

		switch (_sel) {
		case RA_SWITCH:
			// Add the amount of resource used by the application
			res_desc->AddUsed(usages_it->second->value);

			// Append the pointer to the current application descriptor
			// in the map of the applications using the resource
			res_desc->UsedBy(_app);
			break;

		case RA_RELEASE:
			// Subtract the amount of resource once used by the
			// application
			res_desc->SubUsed(usages_it->second->value);

			// Remove the pointer to the current application descriptor
			// in the map of the applications using the resource
			res_desc->NoMoreUsedBy(_app);
			break;
		}
	}
	// If this is a resource release...
	if (_sel == RA_RELEASE)
		// Remove the application from the map of usages
		usages.erase(_app->Name());
}


std::string popNamespaceTemplate(std::string & _next_path) {

	// String to return (head of the path)
	std::string _curr_ns;

	// Find the position of the ID
	int id_pos = _next_path.find_first_of("0123456789");

	if (id_pos == -1) {
		// No ID found
		_curr_ns = _next_path;
		_next_path.clear();
	}
	else {
		// Split
		_curr_ns = _next_path.substr(0, id_pos);
		_next_path = _next_path.substr(id_pos + 1);
	}
	return _curr_ns;
}


std::string const ResourceAccounter::pathTemplate(std::string const & _path) {

	// Resulting template path string
	std::string str_templ;

	// Extract the first node in the resource path
	std::string ns_path	= _path;
	std::string curr_ns = popNamespaceTemplate(ns_path);

	// Iterate over each node in the path
	while (true) {

		if (curr_ns.empty())
			break;
		// Append the current node name
		str_templ += curr_ns;

		// Next node
		curr_ns = popNamespaceTemplate(ns_path);

		// If this is not the last namespace in the path append a "."
		if (!curr_ns.empty())
			str_templ += ".";
	}
	// The template path built
	return str_templ;
}


}   // namespace res

}   // namespace bbque

