/*
 * Copyright (C) 2012  Politecnico di Milano
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "aprox_test.h"

#include "bbque/modules_factory.h"

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
		logger->Debug(" >>> Registering... :%s\n",
				res_names[i].c_str());
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
	//ApplicationProxy::resp_ftr_t stopResp_ftr;
	//ApplicationProxy::pcmdRsp_t pcmdRsp;
	//RTLIB_ExitCode result;
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
	am.CreateEXC("mp3player", 3324, 0, "simple_1Tl2Cl2Pe", 0, true);
	papp = am.GetApplication(3324);
	if (!papp) {
		logger->Error("FAILED: application not started.");
		goto exit_failed;
	}

#if 0
	// Stopping application Execution
	logger->Info("Trying stopping application \"mp3player\"...");
	stopResp_ftr = ap.StopExecution(papp);

	// Waiting for responce from the application
	logger->Info("Waiting for application \"mp3player\" responding...");
	stopResp_ftr.wait();

	// Checkig message responce
	result = stopResp_ftr.get();
	if (result != RTLIB_OK) {
		logger->Error("FAILED: incorrect command execution");
		goto exit_failed;
	}
#endif
	logger->Info("ApplicationProxy TEST SUCCESS\n");
	return;

exit_failed:
	logger->Fatal("ApplicationProxy TEST FAILED\n");
	assert(false);
	return;
}

}   // namespae test

}   // namespace bbque

