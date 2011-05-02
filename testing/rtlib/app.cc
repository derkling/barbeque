/**
 *       @file  dummy_app.cc
 *      @brief  A dummy application which interact with the barbeque RTRM
 *
 * This is a really simple application which could be used as a template to
 * develop real applications interacting with the Barbeque RTRM
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  04/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "app.h"

#include "utility.h"

#include <assert.h>

#define LOG(fmt, ...) LOGGER(COLOR_GREEN, "BBQAPP      ", fmt, ## __VA_ARGS__)

#ifndef DBG
# define DBG(fmt, ...) LOGGER(COLOR_LGRAY, "BBQAPP      ", fmt, ## __VA_ARGS__)
#endif

BbqueApp::BbqueApp(std::string const & name) {
	(void)name;

	LOG("Initializing RPC library...");

	rtlib = RTLIB_Init(name.c_str());
	assert(rtlib);

}

int BbqueApp::RegisterEXC(std::string const & name) {
	(void)name;

	LOG("Registering EXC [%s]...", name.c_str());

	return 0;
}

void BbqueApp::Start() {

	LOG("Starting application...");

}

void BbqueApp::Stop() {

	LOG("Stopping application...");

}




