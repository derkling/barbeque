/**
 *       @file  main.cc
 *      @brief  Tyhe RTRM protorype implementation for 2PARMA EU FP7 project
 *
 * Detailed description starts here.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/11/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include "bbque/barbeque.h"

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>


// Testing Object Class
#include "bbque/object.h"

namespace bbque {
class Test : public Object {
public:
	Test() : Object("Test") {}
	void sayHello() {
		//int i=0;
		std::cout << "Hello" << std::endl;
		DEBUG("Hello Object class");
		logger->Info("This is an info note: %s", "OK");
	}
};
}

// Logger configuration
bool g_log_colored = true;
log4cpp::Category & logger = log4cpp::Category::getInstance("bq");
std::string g_log_configuration = "/tmp/bbque.conf";

int main(int argc, char *argv[])
{
	std::cout << "\t\t.:: Barbeque RTRM (ver. " << g_git_version << ") ::." << std::endl;
	std::cout << "Built: " << __DATE__ << " " << __TIME__ << std::endl;

	// Logger initialization
	try {
		std::cout << "Using logger configuration: " << g_log_configuration << std::endl;
		log4cpp::PropertyConfigurator::configure(g_log_configuration);
	} catch(log4cpp::ConfigureFailure& f) {
		std::cout << "Logger configuration failed: " << f.what() << std::endl;
		return EXIT_FAILURE;
	}
	logger.debug("Logger correctly initialized");
	logger.setPriority(log4cpp::Priority::INFO);

	bbque::Test t;
	t.sayHello();



	return EXIT_SUCCESS;
}

