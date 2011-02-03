/**
 *       @file  resource_manager.cc
 *      @brief  The Barbeque Run-Time Resource Manager
 *
 * This class provides the implementation of the Run-Time Resource Manager
 * (RTRM), which is the main barbeque module implementing its glue code.
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

#include "bbque/resource_manager.h"

#include "bbque/plugin_manager.h"
#include "bbque/modules_factory.h"

namespace bp = bbque::plugins;

namespace bbque {

ResourceManager & ResourceManager::GetInstance() {
	static ResourceManager rtrm;

	return rtrm;
}

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
}

void ResourceManager::Go() {

	// Load a logger module
	bp::PluginManager & pm = bp::PluginManager::GetInstance();
	const bp::PluginManager::RegistrationMap & rm = pm.GetRegistrationMap();

#if 0
	for (bp::PluginManager::RegistrationMap::const_iterator i = rm.begin(); i != rm.end(); ++i) {
	//	monsterTypes_.push_back(i->first);
	}

//	// Dump all the loaded modules
//	for (MonsterTypeVec::iterator i = monsterTypes_.begin(); i != monsterTypes_.end(); ++i) {
//		std::string m = *i;
//		std::cout << m.c_str() << std::endl;
//	}
#endif


	//---------- JustForTest
	// Build a TestModule
	plugins::TestIF * tm = ModulesFactory::GetTestModule("DummyModule");
	if (tm) {
		tm->Test();
	} else {
		std::cerr << "Unable to find a \"DummyObject\" module" << std::endl;
	}

	while (!done) {
		ControlLoop();
	}

}

void ResourceManager::ControlLoop() {
	// Fake control loop implementation
	::sleep(3);
	done = true;
}

} // namespace bbque

