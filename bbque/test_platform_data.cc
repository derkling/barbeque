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

#include "bbque/test_platform_data.h"

#include "bbque/configuration_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/resource_accounter.h"

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
		ResourceAccounter &ra(ResourceAccounter::GetInstance());
		char resourcePath[] = "tile0.cluster256.mem0";
		//                     .............^
		//                            13

		if (platformLoaded)
				return TPD_SUCCESS;

		logger->Warn("Loading TEST platform data");
		logger->Debug("Cluster        : %5d", cm.TPD_ClusterCount());
		logger->Debug("Cluster memory : %5d [MB]", cm.TPD_ClusterMem());
		logger->Debug("PEs per cluster: %5d", cm.TPD_PEsCount());

		// Registering Clusters, per-clusters memory and PEs
		for (uint8_t c = 0; c < cm.TPD_ClusterCount(); ++c) {

				snprintf(resourcePath+13, 8, "%d.mem0", c);
				printf(" >>> Registering... :%s\n", resourcePath);
				ra.RegisterResource(resourcePath, "MB", cm.TPD_ClusterMem());

				for (uint8_t p = 0; p < cm.TPD_PEsCount(); ++p) {
						snprintf(resourcePath+13, 8, "%d.pe%d", c, p);
						printf(" >>> Registering... :%s\n", resourcePath);
						ra.RegisterResource(resourcePath, " ", 100);
				}

		}

		platformLoaded = true;

		return TPD_SUCCESS;
}

} // namespace bbque

