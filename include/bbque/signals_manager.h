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

#ifndef BBQUE_SIGNALS_MANAGER_H_
#define BBQUE_SIGNALS_MANAGER_H_

#include "bbque/config.h"

#include "bbque/utils/utility.h"
#include "bbque/plugins/logger.h"
#include "bbque/resource_manager.h"

#ifdef CONFIG_TARGET_ANDROID
# include <linux/signal.h>
#else
# include <sys/signal.h>
#endif

namespace bbque {

/**
 * @brief A system signal handler
 *
 * This class provides a C++ abstraction for system sygnals.
 */
class SignalHandler {

public:

	/**
	 * The hook method for signal specific handlers
	 */
	virtual int Handler(int signum) = 0;

};


/**
 * @brief A notifyer of a single system signal
 *
 * An event notifier allows to bind a BarbqueRTRM event to a system signal, in
 * such a way that once that signal is received the corresponding BBQ events
 * are notified to the main event processing queue.
 */
class EventNotifier : public SignalHandler {

public:

	EventNotifier(int signum, ResourceManager::controlEvent_t event) :
		sn(signum),
		evt(event),
		asserted(0) {
		}

	virtual int Handler(int signum) {
		assert(signum == sn);
		this->asserted = 1;
		ResourceManager &rm = ResourceManager::GetInstance();
		rm.NotifyEvent(evt);
		return 0;
	}

	sig_atomic_t pending(void) {
		return this->asserted;
	}

private:
	int sn;
	ResourceManager::controlEvent_t evt;
	sig_atomic_t asserted;
};


/**
 * @brief The Barbeque signal management module
 *
 * This class provides a unified interface to register handlers for system
 * signals and to properly dispatch signals to them once a signal is sent to
 * the BarbequeRTRM.
 */
class SignalsManager {

public:

	static SignalsManager & GetInstance();

	/**
	 * @brief Register a SignalHandler for the specified signal number
	 *
	 * Allows to register an handler for a specified signal number
	 * and return a pointer to any pre-existing handler.
	 */
	SignalHandler *RegisterHandler(int signum, SignalHandler *sh);

	/**
	 * @brief Remove a previsously registered handler
	 *
	 * Remove any SignalHandler for the specified signal number.
	 */
	int RemoveHandler(int signum);

private:

	  /**
	   * @brief The logger used by the resource manager.
	   */
	  plugins::LoggerIF *logger;

	  /**
	   * @brief Table of concrete SignalHandlers
	   *
	   * For each signal number applications could register different handler.
	   * NSIG is the number of signals defined in </usr/include/sys/signal.h>.
	   */
	  static SignalHandler *hdlrs[NSIG];

	  /**
	   * @brief Notify a USR1 system event
	   */
	  EventNotifier usr1En;

	  /**
	   * @brief Notify a USR2 system event
	   */
	  EventNotifier usr2En;

	  /**
	   * @brief Notify an INTR system event
	   */
	  EventNotifier intrEn;

	  /**
	   * @brief Notify a QUIT system event
	   */
	  EventNotifier quitEn;

	  /**
	   * 
	   */
	  SignalsManager();

	  /**
	   * 
	   */
	  ~SignalsManager();

	  /**
	   * @brief Entry point adapter installed into <sigaction>
	   */
	  static void Dispatcher(int signum);


};


} // namespace bbque

#endif // BBQUE_SIGNALS_MANAGER_H_
