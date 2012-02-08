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

#ifndef BBQUE_PLUGIN_HELPER_H_
#define BBQUE_PLUGIN_HELPER_H_

#include "bbque/plugins/plugin.h"

#error MUST include headers defining ASSERTS and CHECK

namespace bbque { namespace plugins {

/**
 * @brief A plugins helper class
 *
 * It is designed to work with plugin object classes that implement the
 * PF_CreateFunc and PF_DestroyFunc mandatory functions as static methods.
 */
class PluginHelper {

	struct RegisterParams : public PF_RegisterParams {
		RegisterParams(PF_PluginAPI_Version v,
				PF_CreateFunc cf,
				PF_DestroyFunc df,
				PF_ProgrammingLanguage pl) {

			version = v;
			createFunc = cf;
			destroyFunc = df;
			programmingLanguage = pl;

		}
	};

public:

	PluginHelper(const PF_PlatformServices * _params) :
		params(_params),
		result(ExitPlugin) {
	}

	PF_ExitFunc GetResult() {
		return result;
	}

	template <typename T>
	void registerObject(const char * id,
		PF_ProgrammingLanguage pl = PF_LANG_CPP) {
		PF_PluginAPIVersion v = {1, 0};

		// Version check
		try {
			CHECK (params_->version.major >= v.major)
				<< "Version mismatch. PluginManager version must be "
				<< "at least " << v.major << "." << v.minor;

			RegisterParams rp(v, T::create, T::destroy, pl);
			int32_t rc = params->registerObject(id, &rp);

			CHECK (rc >= 0)
				<< "Registration of object type "
				<< objectType << "failed. "
				<< "Error code=" << rc;
		} catch (...) {
			result_ = NULL;
		}
	}

	static int32_t ExitPlugin() {
		return 0;
	}

private:

	const PF_PlatformServices * params;

	PF_ExitFunc result;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_PLUGIN_HELPER_H_
