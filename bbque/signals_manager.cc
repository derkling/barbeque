/**
 *       @file  signals.h
 *      @brief  The Barbeque Signals Management
 *
 * This class provides the implementation of Signals Management and Signals
 * Handlers for the main Baebque handled signals
 * 
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  05/27/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/signals_manager.h"

#include "bbque/modules_factory.h"

#define SIGNALS_MANAGER_NAMESPACE "bq.sig"

namespace bp = bbque::plugins;

namespace bbque {

SignalHandler *SignalsManager::hdlrs[NSIG];

SignalsManager & SignalsManager::GetInstance() {
	static SignalsManager sm;
	return sm;
}

SignalsManager::SignalsManager() :
	usr1En(SIGUSR1, ResourceManager::BBQ_USR1),
	usr2En(SIGUSR2, ResourceManager::BBQ_USR2),
	intrEn(SIGINT,  ResourceManager::BBQ_EXIT),
	quitEn(SIGQUIT, ResourceManager::BBQ_ABORT) {

	//---------- Get a logger module
	bp::LoggerIF::Configuration conf(SIGNALS_MANAGER_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "SM: Logger module creation FAILED\n");
		assert(logger);
	}

	//---------- Registering handlers
	RegisterHandler(SIGUSR1, &usr1En);
	RegisterHandler(SIGUSR2, &usr2En);
	RegisterHandler(SIGINT,  &intrEn);
	RegisterHandler(SIGQUIT, &quitEn);

	logger->Info("System signals installed, signal catcher thread [%d]",
			gettid());

}

SignalsManager::~SignalsManager() {
}



SignalHandler *SignalsManager::RegisterHandler(int signum, SignalHandler *sh) {
	struct sigaction sa;

	// Save the old signal handler
	SignalHandler *old = SignalsManager::hdlrs[signum];

	// Store the new signal handler
	SignalsManager::hdlrs[signum] = sh;

	// Delegate SignalDispatcher method to handle this signal
	sa.sa_handler = SignalsManager::Dispatcher;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction (signum, &sa, 0);

	return old;
}

void SignalsManager::Dispatcher(int signum) {

	// Ignore empty signals
	if (!SignalsManager::hdlrs[signum])
		return;

	// Dispatch the signal to the registered handler
	SignalsManager::hdlrs[signum]->Handler(signum);

}

} // namespace bbque

