/**
 *       @file  plugin_helper.h
 *      @brief  This provides an helper class that takes the drudge out of
 *				writing the plugin glue code
 *
 * It is designed to work with plugin object classes that implement the
 * PF_CreateFunc and PF_DestroyFunc mandatory functions as static methods.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  01/27/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef BBQUE_PLUGIN_HELPER_H_
#define BBQUE_PLUGIN_HELPER_H_

#include "bbque/plugins/plugin.h"

#error MUST include headers defining ASSERTS and CHECK

namespace bbque { namespace plugins {

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

