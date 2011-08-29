/**
 *       @file  signals.h
 *      @brief  The Barbeque Signals Management
 *
 * This class provides the implementation of Signals Management and Signals
 * Handlers for the main Baebque handled signals
 * 
 *     @author  Patrick Bellasi (derkling), derkling@google.com
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

#ifndef BBQUE_SIGNALS_MANAGER_H_
#define BBQUE_SIGNALS_MANAGER_H_

#include "bbque/config.h"

#include "bbque/utils/utility.h"
#include "bbque/plugins/logger.h"
#include "bbque/resource_manager.h"

#include <sys/signal.h>

namespace bbque {

class SignalHandler {

public:

	/**
	 * The hook method for signal specific handlers
	 */
	virtual int Handler(int signum) = 0;

};


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
	   * #brief Entry point adapter installed into <sigaction>
	   */
	  static void Dispatcher(int signum);


};


} // namespace bbque

#endif // BBQUE_SIGNALS_MANAGER_H_

