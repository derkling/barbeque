/**
 *       @file  platform_services.h
 *      @brief  The services provide to plugins by the barbeque core
 *
 * This class provides a set of services to barbeuqe modules throughout the
 * single InvokceServices method.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  02/01/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
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

