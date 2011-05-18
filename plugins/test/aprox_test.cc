/**
 *       @file  aprox_test.cc
 *      @brief  A test for the ApplicationProxy module
 *
 * This provides a class to test the implementation of the ApplicationPorxy
 * module. This test evaluate both functional and performance aspects of the
 * implementation.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  05/06/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "aprox_test.h"

#include "bbque/modules_factory.h"

#include "bbque/res/resource_accounter.h"
#include "bbque/application_manager.h"
#include "bbque/application_proxy.h"
#include "bbque/system_view.h"

#include <iomanip>

namespace ba = bbque::app;
namespace bp = bbque::plugins;
namespace br = bbque::res;

namespace bbque { namespace plugins {


ApplicationProxyTest::ApplicationProxyTest() {

	// Get a logger
	std::string logger_name(TEST_NAMESPACE APROX_NAMESPACE);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = std::unique_ptr<bp::LoggerIF>(
			ModulesFactory::GetLoggerModule(std::cref(conf)));
	assert(logger);

}

ApplicationProxyTest::~ApplicationProxyTest() {

}

// ===================[ Plugin interfaces ]====================================

void * ApplicationProxyTest::Create(PF_ObjectParams *) {
	return new ApplicationProxyTest();
}

int32_t ApplicationProxyTest::Destroy(void * plugin) {
	if (!plugin)
		return -1;
	delete (ApplicationProxyTest *) plugin;
	return 0;
}


// ===================[ Test functions ]=======================================

std::vector<std::string> res_names = {
	"mem0",
	"dma0",
	"arch.mem0",
	"arch.tile0.mem0",
//--- Cluster 0 (with 2PEs)
	"arch.tile0.cluster0",
	"arch.tile0.cluster0.mem0",
	"arch.tile0.cluster0.dma0",
	"arch.tile0.cluster0.pe0",
	"arch.tile0.cluster0.pe0.mem0",
	"arch.tile0.cluster0.pe1",
	"arch.tile0.cluster0.pe1.mem0",
//--- Cluster 1 (with 2PEs)
	"arch.tile0.cluster1",
	"arch.tile0.cluster1.mem0",
	"arch.tile0.cluster1.dma0",
	"arch.tile0.cluster1.pe0",
	"arch.tile0.cluster1.pe0.mem0",
	"arch.tile0.cluster1.pe1",
	"arch.tile0.cluster1.pe1.mem0"
};

std::vector<std::string> res_types = {
	"SDRAM memory",
	"bus",
	"SDRAM memory",
	"SDRAM memory",
//--- Cluster 0 (with 2PEs)
	"cluster",
	"SDRAM memory",
	"bus",
	"cpu",
	"memory",
	"cpu",
	"memory",
//--- Cluster 1 (with 2PEs)
	"cluster",
	"SDRAM memory",
	"bus",
	"cpu",
	"memory",
	"cpu",
	"memory"
};

std::vector<std::string> res_units = {
	"Mb",
	"Mbps",
	"Mb",
	"Mb",
//--- Cluster 0 (with 2PEs)
	"1",
	"Mb",
	"Mbps",
	"1",
	"kb",
	"1",
	"kb",
//--- Cluster 1 (with 2PEs)
	"1",
	"Mb",
	"Mbps",
	"1",
	"kb",
	"1",
	"kb"
};

std::vector<uint64_t> res_totals = {
	256,  // System RAM
	200,  // DMA system bus
	32,   // Memory architecture level
	16,   // Memory inter-cluster level
//--- Cluster 0 (with 2PEs)
	2,    // Number of PE in cluster 0
	8,    // Memory in cluster 0
	50,   // DMA for cluster 0
	1,    // Num of pe0 in cluster0
	512,  // cluster0.pe0.mem0
	1,    // Num of pe1 in cluster1
	512,  // cluster0.pe1.mem0
//--- Cluster 1 (with 2PEs)
	2,    // Number of PE in cluster 0
	8,    // Memory in cluster 0
	50,   // DMA for cluster 0
	1,    // Num of pe0 in cluster0
	512,  // cluster1.pe0.mem0
	1,    // Num of pe1 in cluster1
	512  // cluster1.pe1.mem0

};


int ApplicationProxyTest::RegisterSomeResources() {
	// Get ResourceAccounter instance
	ResourceAccounter &ra(ResourceAccounter::GetInstance());

	// Start register resources
	for (uint16_t i=0; i < res_names.size(); ++i) {
		logger->Debug(" >>> Registering... :%s\n", res_names[i].c_str());
		ra.RegisterResource(res_names[i], res_units[i], res_totals[i]);
	}
	// Print a tree-like view of the resources
	ra.TreeView();
	return 0;
}


int ApplicationProxyTest::PrintResourceAvailabilities() {
	// Get ResourceAccounter instance
	ResourceAccounter &ra(ResourceAccounter::GetInstance());

	std::cout << "\n______________________| Resource availabilities "
		"|___________________\n" << std::endl;

	// Print resource availabilities info
	for (uint16_t i = 0; i < res_names.size(); ++i) {
		std::cout << std::setw(50) << std::left << res_names[i].c_str()
			<< "| " << std::setw(15) << std::right
			<< ra.Available(res_names[i])
			<< " |" << std::endl;
	}
	std::cout << "__________________________________________________"
		"__________________\n" << std::endl;

	return 0;
}


// ===================[ Start the test ]=======================================

void ApplicationProxyTest::Test() {
	bbque::ApplicationManager &am(bbque::ApplicationManager::GetInstance());
	bbque::ApplicationProxy &ap(bbque::ApplicationProxy::GetInstance());
	bbque::SystemView &sv(bbque::SystemView::GetInstance());
	ApplicationProxy::resp_ftr_t stopResp_ftr;
	ApplicationProxy::pcmdRsp_t pcmdRsp;
	AppPtr_t papp;

	logger->Info("ApplicationProxy TEST STARTED");

	// Platform setup
	if (RegisterSomeResources()) {
		logger->Error("FAILED: resources registration");
		goto exit_failed;
	}
	if (PrintResourceAvailabilities()) {
		logger->Error("FAILED: resources registration");
		goto exit_failed;
	}

	// Starting the ApplicationProxy service
	ap.Start();

	// Periodically stop READY applications
	logger->Debug("Killing READY application loop...");
	for (uint8_t i=10; i; i--) {
		::sleep(6);
		logger->Debug("Registered READY applications: %d",
			sv.ApplicationsReady()->size());
	}

	return;

	// Start an application to use for testing
	am.CreateEXC("mp3player", 3324, 0, "simple_1Tl2Cl2Pe");
	papp = am.GetApplication(3324);
	if (!papp) {
		logger->Error("FAILED: application not started.");
		goto exit_failed;
	}

	// Stopping application Execution
	logger->Info("Trying stopping application \"mp3player\"...");
	stopResp_ftr = ap.StopExecution(papp);

	// Waiting for responce from the application
	logger->Info("Waiting for application \"mp3player\" responding...");
	stopResp_ftr.wait();

	// Checkig message responce
	pcmdRsp = stopResp_ftr.get();
	if (pcmdRsp->result != RTLIB_OK) {
		logger->Error("FAILED: incorrect command execution");
		goto exit_failed;
	}

	logger->Info("ApplicationProxy TEST SUCCESS\n");
	return;

exit_failed:
	logger->Fatal("ApplicationProxy TEST FAILED\n");
	assert(false);
	return;
}

}   // namespae test

}   // namespace bbque

