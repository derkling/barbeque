/**
 *       @file  coreint_test.cc
 *      @brief  CoreInteractions plugin implementation
 *
 * This provides the implementation of the test plugin CoreInteractions
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

#include "coreint_test.h"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <vector>

#include "bbque/modules_factory.h"
#include "bbque/rtlib.h"
#include "bbque/app/recipe.h"
#include "bbque/app/working_mode.h"
#include "bbque/app/plugin_data.h"
#include "bbque/res/resources.h"
#include "bbque/utils/timer.h"

#define NUM_RECIPES 5

// Enabling test macros
#define SINGLE_APP_TEST_ENABLED	1
#define SCHEDU_APP_TEST_ENABLED	1

namespace ba = bbque::app;
namespace bp = bbque::plugins;
namespace br = bbque::res;
namespace bu = bbque::utils;

namespace bbque { namespace plugins {

/** Map of application descriptor shared pointers */
typedef std::map<uint32_t, AppPtr_t> AppsMap_t;

 /** The RNG used for testcase initialization. */
std::mt19937 rng_engine(time(0));

// Test set
std::vector<std::string> res_names = {
	// System-wide resources
	"mem0",
	"dma0",
	// Platform resources
	"arch.mem0",
	"arch.tile0.mem0",
	// Cluster level memories
	"arch.tile0.cluster0.mem0",
	"arch.tile0.cluster1.mem0",
	"arch.tile0.cluster2.mem0",
	"arch.tile0.cluster3.mem0",
	// Cluster level DMAs
	"arch.tile0.cluster0.dma0",
	"arch.tile0.cluster1.dma0",
	"arch.tile0.cluster2.dma0",
	"arch.tile0.cluster3.dma0",
	// Processing elements
	"arch.tile0.cluster0.pe0",
	"arch.tile0.cluster0.pe1",
	"arch.tile0.cluster0.pe2",
	"arch.tile0.cluster0.pe3",
	"arch.tile0.cluster0.pe4",
	"arch.tile0.cluster0.pe5",
	"arch.tile0.cluster1.pe0",
	"arch.tile0.cluster1.pe1",
	"arch.tile0.cluster1.pe2",
	"arch.tile0.cluster1.pe3",
	"arch.tile0.cluster2.pe0",
	"arch.tile0.cluster2.pe1",
	"arch.tile0.cluster2.pe2",
	"arch.tile0.cluster2.pe3",
	"arch.tile0.cluster3.pe0",
	"arch.tile0.cluster3.pe1",
	"arch.tile0.cluster3.pe2",
	"arch.tile0.cluster3.pe3",
};

std::vector<std::string> res_units = {
	"Mb",
	"Mbps",

	"Mb",
	"Mb",

	"Kb",
	"Kb",
	"Kb",
	"Kb",

	"Mbps",
	"Mbps",
	"Mbps",
	"Mbps",

	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1"
};

std::vector<uint64_t> res_totals = {
	32, 	// System RAM
	200,  	// DMA system bandwidth

	4,   	// Memory architecture level
	2,   	// Memory inter-cluster level

	256, 	// Memory intra-cluster level
	256,
	256,
	256,

	50, 	// DMA intra-cluster bandwidht
	50,
	50,
	50,

	1, 	// Number of processing elements
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1
};

std::vector<std::string> rsrcSearchPaths = {
	"arch.tile0.mem",
	"arch.tile0.cluster",
	"arch.tile.cluster.pe",
	"dma",
	"spi",
	RSRC_CLUSTER,
	"arch.tile.cluster2.pe0"
};


CoreInteractionsTest::CoreInteractionsTest():
	sv(SystemView::GetInstance()),
	am(ApplicationManager::GetInstance()),
	ra(ResourceAccounter::GetInstance()){

	// Get a logger
	std::string logger_name(TEST_NAMESPACE COREINT_NAMESPACE);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	assert(logger);

	if (logger)
		logger->Debug("CoreInteractionsTest: %s", logger_name.c_str());
}


CoreInteractionsTest::~CoreInteractionsTest() {

}

// ===================[ Plugin interfaces ]====================================

void * CoreInteractionsTest::Create(PF_ObjectParams *) {
	return new CoreInteractionsTest();
}


int32_t CoreInteractionsTest::Destroy(void * plugin) {
	if (!plugin)
		return -1;
	delete (CoreInteractionsTest *) plugin;
	return 0;
}


void RegisterSomeResources() {
	ResourceAccounter & ra(ResourceAccounter::GetInstance());
	std::cout << "names=" << res_names.size()
		<< " units=" << res_units.size()
		<< " totals=" << res_totals.size()
		<< std::endl;

	bu::Timer _t(true);
	// Start register resources
	for (uint16_t i=0; i < res_names.size(); ++i) {
		printf(" >>> Registering... :%s\n", res_names[i].c_str());
		ra.RegisterResource(res_names[i], res_units[i],
				res_totals[i]);
	}

	_t.stop();
	std::cout << "\nResources registered in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	// Print a tree-like view of the resources
	ra.TreeView();
	std::cout << "Press a key..." << std::endl;
	getchar();
}


// ===================[ Print functions ]=======================================

void PrintResourceAvailabilities(SystemView & sv) {

	std::cout
		<<
		"\n______________________| Resource availabilities |___________________\n"
		<< std::endl;

	// Print resource availabilities info
	for (uint16_t i = 0; i < res_names.size(); ++i) {
		std::cout << std::setw(50) << std::left << res_names[i].c_str() << "| "
			<< std::setw(15) << std::right <<
			sv.ResourceAvailability(res_names[i])
			<< " |" << std::endl;
	}
	std::cout
		<<
		"____________________________________________________________________\n"
		<< std::endl;
}


void PrintScheduleInfo(AppPtr_t & papp) {

	if (!papp) {
		std::cout
			<< "Null application descriptor pointer passed"
			<< std::endl;
		return;
	}

	if (!(papp->CurrentAWM())) {
		std::cout << "! - Current AWM not set - " << std::endl;
	}
	else {
		std::cout
			<< papp->Name().c_str() << ": "
			<< "Curr sched = "
			<< "AWM" << papp->CurrentAWM()->Id()
			<< " " << papp->CurrentAWM()->Name()
			<< " | State " << papp->State() << std::endl;
	}

	if (papp->NextAWM().get() == NULL) {
		std::cout << "[!] Next AWM not set" << std::endl;
	}
	else {
		std::cout
			<< papp->Name().c_str() << ": "
			<< "Next sched = "
			<< "AWM" << papp->NextAWM()->Id()
			<< " " << papp->NextAWM()->Name() << std::endl;
	}

	std::cout << "Press a key..." << std::endl;
	getchar();
}


int PrintWorkingModesInfo(AppPtr_t papp) {

	// Application descriptor pointer is valid?
	if (!papp) {
		std::cout << "Application is NULL" << std::endl;
		return 1;
	}

	// Get all the enabled working modes
	AwmPtrList_t const * awms = papp->WorkingModes();
	if (awms->empty()) {
		std::cout << "Cannot find any working modes" << std::endl;
		return 2;
	}

	// For each working mode...
	AwmPtrList_t::const_iterator wm_it = awms->begin();
	AwmPtrList_t::const_iterator endm = awms->end();
	for (; wm_it != endm; ++wm_it) {

		fprintf(stderr, "\n\n *** AWM%d [ %s ] (value = %.2f)"
				"%d resource usages ***\n",
				(*wm_it)->Id(),
				(*wm_it)->Name().c_str(),
				(*wm_it)->Value(),
				(int)(*wm_it)->ResourceUsages().size());

		std::cout
			<< "\n--------------------------------[ "
			<< (*wm_it)->Id() << " "
			<< (*wm_it)->Name()
			<< " ]----------------------------" << std::endl;

		// Casting to WorkingModeStatusIF
		AwmPtr_t c_awm = *wm_it;

		UsagesMap_t::const_iterator res_it = c_awm->ResourceUsages().begin();
		UsagesMap_t::const_iterator res_end = c_awm->ResourceUsages().end();

		// Print resource usage values
		for ( ; res_it != res_end; ++res_it) {
			std::cout <<
				std::setw(50) << std::left	<< (res_it->first).c_str() << ":"
				<< std::setw(15) << std::right
				<< c_awm->ResourceUsageValue(res_it->first) << " |" << std::endl;
		}

		std::cout
			<< "-------------------------------------------------------------"
			<< "-----" << std::endl;
	}

	AwmPtr_t l_awm = papp->LowValueAWM();
	std::cout
		<< l_awm->Name()
		<< " is the working mode with the lowest value [" << l_awm->Value()
		<< "]" << std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();

	return 0;
}


// ===================[ Test methods ]========================================

void testPathTemplateSearch(SystemView & sv,
		std::vector<std::string> & rsrc_paths) {

	bu::Timer _t(true);
	std::cout << ".........: Test resource template search :......\n"
		<< std::endl;

	// Make a path template based search of the given resources set
	for (std::vector<std::string>::iterator it = rsrc_paths.begin();
			it != rsrc_paths.end(); ++it) {

		std::cout << std::setw(40) << (*it) << ": ";
		if (sv.ExistResource((*it)))
			std::cout << std::right << "FOUND" << std::endl;
		else
			std::cout << "NOT FOUND" << std::endl;
	}

	_t.stop();
	std::cout << "\nSearch finished in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();
}


void testResourceSetSearch(SystemView & sv,
		std::vector<std::string> & rsrc_paths) {

	bu::Timer _t(true);
	std::cout << "_________| Test resource groups search |_______\n"
		<< std::endl;

	// List of matches
	std::list<br::ResourcePtr_t> res_match;

	// Make a path template based search of the given resources set
	for (std::vector<std::string>::iterator rsrc_it = rsrc_paths.begin();
			rsrc_it != rsrc_paths.end(); ++rsrc_it) {

		// How many matchings ?
		res_match = sv.GetResources((*rsrc_it));
		std::cout << "[" << (*rsrc_it) << "] matchings : "
			<< res_match.size() << std::endl;

		// Print out one by one
		br::ResourcePtrList_t::iterator it = res_match.begin();
		br::ResourcePtrList_t::iterator end = res_match.end();
		for(; it != end; ++it)
			std::cout << "\t" << (*it)->Name().c_str() << std::endl;

		// Print amounts information
		std::cout
			<< "\tUSED: "
			<< sv.ResourceUsed((*rsrc_it))
			<< "\tTOT: "
			<< sv.ResourceTotal((*rsrc_it))
			<< "\tAVAIL: "
			<< sv.ResourceAvailability((*rsrc_it))
			<< std::endl << std::endl;
	}

	_t.stop();
	std::cout << "\nSearch finished in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();
}


void GetClusteredInfo(SystemView & sv,
		std::vector<std::string> & rsrc_paths) {

	bu::Timer _t(true);
	std::cout
		<< "-----------------| Resource clustering factor |----------------"
		<< std::endl;

	// Test for the clustering factor computation
	for (std::vector<std::string>::iterator it = rsrc_paths.begin();
			it != rsrc_paths.end(); ++it) {

		std::cout
			<< std::setw(40) << std::left
			<< (*it) << " CF = " << sv.ResourceClusterFactor((*it))
			<< std::endl;
	}

	_t.stop();
	std::cout << "\nSearch finished in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();
}


void CoreInteractionsTest::testScheduleSwitch(AppPtr_t & papp,
		uint8_t wm_id,
		double ov_time) {
	Application::ExitCode_t result;
	(void) ov_time;

	// Get working mode wm1
	if (papp.get() == NULL) {
		std::cout << "Null application descriptor pointer passed"
			<< std::endl;
		return;
	}

	// Get working mode descriptor related to "wm"
	AwmPtr_t d_wm = papp->GetWorkingMode(wm_id);
	if (d_wm.get() == NULL) {
		std::cout << "Working mode ID=" << wm_id << " not found"
			<< std::endl;
		return;
	}

	// Do a resource binding!
	UsagesMapPtr_t rsrc_binds(new UsagesMap_t);
	d_wm->BindResource("cluster", RSRC_ID_ANY, 1, rsrc_binds);
	logger->Debug("Usages / Binds = %d / %d",
			d_wm->ResourceUsages().size(),
			rsrc_binds->size());

	// Let's set next schedule for the application above
	// the binding is set by ScheduleRequest.
	RViewToken_t vtok;
	ra.GetView("sched.pol.fake", vtok);
	result = papp->ScheduleRequest(d_wm, rsrc_binds, vtok);
	if (result != Application::APP_WM_ACCEPTED) {
		logger->Error("AWM Rejected");
	}

	PrintScheduleInfo(papp);

	if (papp->State() != Application::SYNC)
		return;

	// Now switch!
	am.SyncCommit(papp);
	PrintScheduleInfo(papp);
	ra.PutView(vtok);
}


void CoreInteractionsTest::testApplicationLifecycle(AppPtr_t & papp) {

	am.EnableEXC(papp);

	logger->Info("_____ Print out working modes details");
	PrintWorkingModesInfo(papp);

	logger->Info("_____  Get the application descriptor");
	AppPtr_t app_conf(am.GetApplication(3324, 0));

	logger->Info("_____ Simulate a schedulation 1");
	testScheduleSwitch(app_conf, 1, 0.381);
	PrintResourceAvailabilities(sv);

	logger->Info("_____ Simulate a schedulation 2...");
	testScheduleSwitch(app_conf, 2, 0.445);
	PrintResourceAvailabilities(sv);

	logger->Info("_____ Come back to awm  1");
	testScheduleSwitch(app_conf, 1, 0.409);
	PrintResourceAvailabilities(sv);

	logger->Info("_____ Stop application");
	ApplicationManager::ExitCode_t result = am.DestroyEXC(3324);
	if (result == ApplicationManager::AM_SUCCESS)
		logger->Info("Application correctly exited.");
	else
		logger->Info("Error: Application didn't exit correctly"
				" [ExitCode = %d]", result);
	PrintResourceAvailabilities(sv);
}


void CoreInteractionsTest::testScheduling() {

	// Is there a Scheduling Policy ?
	plugins::SchedulerPolicyIF * scheduler =
		ModulesFactory::GetSchedulerPolicyModule();

	if (!scheduler) {
		logger->Error("SchedulerPolicy not found");
		getchar();
		return;
	}

	logger->Info("~~~~~~~~~~~~~ Scheduling in progress ~~~~~~~~~~~~");
	bu::Timer _t(true);

	// Schedule
	scheduler->Schedule(sv);
	_t.stop();

	// Print scheduling report
	AppsMap_t::const_iterator  app_it(am.Applications()->begin());
	AppsMap_t::const_iterator  app_end(am.Applications()->end());
	for (; app_it != app_end; ++app_it) {

		AwmPtr_t sched_awm(app_it->second->NextAWM());
		if (!sched_awm) {
			logger->Warn("%s not scheduled", app_it->second->StrId());
			continue;
		}

		logger->Info("[%s] scheduled in AWM{%d} clusters = %s",
				app_it->second->StrId(),
				sched_awm->Id(),
				sched_awm->ClusterSet().to_string().c_str());
	}

	logger->Info("~~~~~~~~~~~~~ Scheduling finishd ~~~~~~~~~~~~");
	logger->Info("time = %4.4f ms", _t.getElapsedTime());
	getchar();
}

void CoreInteractionsTest::testSyncResourcesUpdate() {

	// Start a sync session for resources acquisition
	ra.SyncStart();

	AppsUidMap_t::const_iterator sync_app_it(
			sv.Applications(ApplicationStatusIF::SYNC)->begin());

	// Get all the applications to syncronize
	for (; sync_app_it != sv.Applications(ApplicationStatusIF::SYNC)->end();
			++sync_app_it) {

		if (sync_app_it->second->SyncState() == ApplicationStatusIF::BLOCKED)
			continue;

		// Acquire resources for the application/ExC
		ra.SyncAcquireResources(sync_app_it->second);

		// TODO: Bug to fix on am.SyncCommit().
		// Beware: ithout this call we cannot perform other runs
		// ---------------------------------------------------------
		// Commit the change of state + AWM to the ApplicationManager
		//am.SyncCommit(sync_app_it->second);
	}

	// Commit the session
	ra.SyncCommit();
}

void CoreInteractionsTest::testStartApplications(uint16_t num) {

	logger->Info("______ Simulate the start of %d applications _____", num);
	std::string recipe_name("r1_platA");

	for (int i = 0; i < num; ++i) {
	    // Recipe index
		std::uniform_int_distribution<uint16_t> dist_pt(1, NUM_RECIPES);
		uint16_t recipe_idx = dist_pt(rng_engine);

		// Recipe name
		std::stringstream ss;
		ss << recipe_idx;
		size_t pos = recipe_name.find_first_of("0123456789", 0);
		recipe_name.replace(pos, pos, ss.str());

		// Application name
		std::string app_name("app");
		std::stringstream ss2;
		ss2 << i;
		app_name += ss2.str();

		// Start!
		AppPtr_t papp(am.CreateEXC(app_name, 1000+i, 0, recipe_name));
		if (papp)
			am.EnableEXC(papp);
	}
	logger->Info("READY Execution context: %d",
			am.Applications(Application::READY)->size());
}


void CoreInteractionsTest::testConstraints(AppPtr_t & papp) {
	RTLIB_Constraint cstr;

	// Upper bound to AWM 2
	cstr.awm = 2;
	cstr.add = true;
	cstr.type = RTLIB_ConstraintType::LOW_BOUND;

	PrintWorkingModesInfo(papp);
	logger->Info("Constraint: ADD Awm = %d Bound = %d", cstr.awm, cstr.type);
	papp->SetWorkingModeConstraint(cstr);
	PrintWorkingModesInfo(papp);

	// Shrink... upper bound to AWM 1
	cstr.awm = 1;
	cstr.type = RTLIB_ConstraintType::UPPER_BOUND;
	logger->Info("Constraint: ADD Awm = %d Bound = %d", cstr.awm, cstr.type);
	papp->SetWorkingModeConstraint(cstr);
	PrintWorkingModesInfo(papp);

	// Remove the last constraint
	papp->ClearWorkingModeConstraint(cstr.type);
	logger->Info("Constraint: REMOVED");
	PrintWorkingModesInfo(papp);
	getchar();

	// Exact value
	cstr.awm = 1;
	cstr.type = RTLIB_ConstraintType::EXACT_VALUE;
	logger->Info("Constraint: ADD Awm = %d  = %d", cstr.awm, cstr.type);
	papp->SetWorkingModeConstraint(cstr);
	PrintWorkingModesInfo(papp);

	// Remove the last constraint
	papp->ClearWorkingModeConstraint(cstr.type);
	logger->Info("Constraint: REMOVED");
	PrintWorkingModesInfo(papp);
}


// =======================================[ Start the test ]==================

void CoreInteractionsTest::Test() {
	logger->Debug("....: CoreInteractions Test starting :.....\n");

	// Resources
	RegisterSomeResources();
	PrintResourceAvailabilities(sv);

	// Some resource search test
	testPathTemplateSearch(sv, rsrcSearchPaths);
	testResourceSetSearch(sv, rsrcSearchPaths);

	// TODO: Feature to fix or drop...
	GetClusteredInfo(sv, rsrcSearchPaths);

#if SINGLE_APP_TEST_ENABLED
	// Create an Execution Context
	am.CreateEXC("mp3player", 3324, 0, "r1_platA", 0, true);
	AppPtr_t test_app(am.GetApplication(3324, 0));
	if (!test_app) {
		logger->Error("Application not started.");
		return;
	}
//	testApplicationLifecycle(test_app);
	testConstraints(test_app);

	// Stop application
/*	ApplicationManager::ExitCode_t result = am.DestroyEXC(3324);
	if (result == ApplicationManager::AM_SUCCESS)
		logger->Info("Application correctly exited.");
	else
		logger->Info("Error: Application didn't exit correctly"
				" [ExitCode = %d]", result);
*/
	// Plugin specific data
	std::string * auth =
		(static_cast<std::string *>(
					(test_app->GetAttribute("YaMCa", "author")).get()));
	if (auth)
		logger->Info("Plugin YaMCa: <author> : %s", auth->c_str());

	PrintResourceAvailabilities(sv);
#endif

#if SCHEDU_APP_TEST_ENABLED
	// Start N applications
	testStartApplications(10);
	logger->Debug("Applications loaded = %d",
			sv.ApplicationsReady()->size());
	getchar();

	// Scheduler test
	testScheduling();
	testSyncResourcesUpdate();
	PrintResourceAvailabilities(sv);

	// Stop applications
	AppsMap_t::const_iterator apps_it(sv.ApplicationsRunning()->begin());
	AppsMap_t::const_iterator end_apps(sv.ApplicationsRunning()->end());
	for (; apps_it != end_apps; ++apps_it)
		am.DestroyEXC(apps_it->second);
#endif
}

}   // namespace test

}   // namespace bbque

