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

