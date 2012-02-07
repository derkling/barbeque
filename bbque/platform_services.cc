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

#include "bbque/platform_services.h"

#include "bbque/configuration_manager.h"
#include "bbque/plugin_manager.h"

namespace bp = bbque::plugins;
namespace po = boost::program_options;

namespace bbque {

PlatformServices::PlatformServices() {
}

PlatformServices::~PlatformServices() {
}

PlatformServices & PlatformServices::GetInstance() {
	static PlatformServices instance;
	return instance;
}

bool PlatformServices::CheckRequest(PF_PlatformServiceID id,
		PF_ServiceData & data) {
	bp::PluginManager & pm = bp::PluginManager::GetInstance();

	PF_ProgrammingLanguage request_lang =
		pm.GetModuleLanguage(std::string(data.id));

	if (request_lang == PF_LANG_CPP)
		return true;

	// Allow only pure C based services to C coded plugins
	if (id<PF_SERVICE_C_BASED_COUNT)
		return true;

	return false;
}

int32_t PlatformServices::ServiceDispatcher(PF_PlatformServiceID id,
		PF_ServiceData & data) {
	PlatformServices ps = PlatformServices::GetInstance();

	if (!ps.CheckRequest(id, data))
		return PF_SERVICE_WRONG;

	switch (id) {
	case PF_SERVICE_CONF_DATA:
		return ps.ServiceConfData(data);
	default:
		return PF_SERVICE_UNDEF;
	}

	return PF_SERVICE_UNDEF;
}

int32_t PlatformServices::ServiceConfData(PF_ServiceData & data) {
	ConfigurationManager & cm = ConfigurationManager::GetInstance();
	po::options_description const & opts_desc =
		 std::cref(*(((PF_Service_ConfDataIn*)data.request)->opts_desc));
	po::variables_map & opts =
		 std::ref(*(((PF_Service_ConfDataOut*)data.response)->opts_value));

	fprintf(stdout, "PS: ServiceConfData ===> '%s'\n", data.id);
	cm.ParseConfigurationFile(opts_desc, opts);

	return PF_SERVICE_DONE;
}


} // namespace bbque

