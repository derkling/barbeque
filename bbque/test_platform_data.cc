/**
 *       @file  test_platform_data.cc
 *      @brief  A dummy loader of test platform data
 *
 * This file provides a descritpion of platform resoruces suitable for testing
 * purposes while a real implementation of the ResourceAbstraction module is
 * missing. The platform defined by this module can be configured using
 * command lines parameters to define the number of clusters, PEs for each
 * cluster, the amount of cluster-shared memory and the amount of PE-private
 * memory.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  06/22/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/test_platform_data.h"

#include "bbque/configuration_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/res/resource_accounter.h"

namespace br = bbque::res;

namespace bbque {

TestPlatformData & TestPlatformData::GetInstance() {
	static TestPlatformData tpd;
	return tpd;
}


TestPlatformData::TestPlatformData() :
		platformLoaded(false) {

	//---------- Get a logger module
	std::string logger_name(TEST_PLATFORM_DATA_NAMESPACE);
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "TPD: Logger module creation FAILED\n");
		assert(logger);
	}

}

TestPlatformData::~TestPlatformData() {
}

TestPlatformData::ExitCode_t
TestPlatformData::LoadPlatformData() {
		ConfigurationManager &cm(ConfigurationManager::GetInstance());
		br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
		char resourcePath[] = "arch.tile0.cluster256.mem0";
		//                     ..................^
		//                            18

		if (platformLoaded)
				return TPD_SUCCESS;

		logger->Warn("Loading TEST platform data");
		logger->Debug("Cluster        : %5d", cm.TPD_ClusterCount());
		logger->Debug("Cluster memory : %5d [MB]", cm.TPD_ClusterMem());
		logger->Debug("PEs per cluster: %5d", cm.TPD_PEsCount());

		// Registering Clusters, per-clusters memory and PEs
		for (uint8_t c = 0; c < cm.TPD_ClusterCount(); ++c) {

				snprintf(resourcePath+18, 8, "%d.mem0", c);
				printf(" >>> Registering... :%s\n", resourcePath);
				ra.RegisterResource(resourcePath, "MB", cm.TPD_ClusterMem());

				for (uint8_t p = 0; p < cm.TPD_PEsCount(); ++p) {
						snprintf(resourcePath+18, 8, "%d.pe%d", c, p);
						printf(" >>> Registering... :%s\n", resourcePath);
						ra.RegisterResource(resourcePath, " ", 1);
				}

		}

		platformLoaded = true;

		return TPD_SUCCESS;
}

} // namespace bbque

