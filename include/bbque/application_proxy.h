/**
 *       @file  application_proxy.h
 *      @brief  A proxy to communicate with registered applications
 *
 * Definition of the class used to communicate with Barbeque managed
 * applications. From the RTRM prespective, each class exposes a set of
 * functionalities which could be accessed using methods defined by this
 * proxy. Each call requires to specify the application to witch it is
 * addressed and the actual parameters.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  04/18/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_APPLICATION_PROXY_H_
#define BBQUE_APPLICATION_PROXY_H_

#include "bbque/plugins/logger.h"
#include "bbque/plugins/rpc_channel.h"
#include "bbque/rtlib/rpc_messages.h"

#include <thread>
#include <memory>
#include <map>

using namespace bbque::plugins;
using namespace bbque::rtlib;

namespace bbque {

class ApplicationProxy {

private:

	std::thread dispatcher_thd;

	plugins::LoggerIF *logger;

	plugins::RPCChannelIF *rpc;

public:

	static ApplicationProxy & GetInstance();

	void Start();

	~ApplicationProxy();

private:

	typedef RPCChannelIF::rpc_msg_ptr_t pchMsg_t;

	//typedef std::shared_ptr<pchMsg_t> pmsg_t;

	typedef struct exeCtx {
		std::thread exe;
		pchMsg_t pmsg;
		pid_t pid;
	} exeCtx_t;

	typedef std::shared_ptr<exeCtx_t> pexeCtx_t;

	typedef std::map<rpc_msg_type_t, pexeCtx_t> exeCtxMap_t;

	exeCtxMap_t exeCtxMap;

	std::mutex exeCtxMap_mtx;


	std::mutex trdStatus_mtx;

	std::condition_variable trdStatus_cv;


	typedef struct conCtx {
		pid_t app_pid;
		RPCChannelIF::plugin_data_t pd;
	} conCtx_t;

	typedef std::shared_ptr<conCtx_t> pconCtx_t;

	typedef std::map<pid_t, pconCtx_t> conCtxMap_t;

	conCtxMap_t conCtxMap;

	std::mutex conCtxMap_mtx;


	ApplicationProxy();

	rpc_msg_type_t GetNextMessage(pchMsg_t & pmsg);

	void CompleteTransaction(pchMsg_t & pmsg);

#define DETACHED_THREAD
#ifdef DETACHED_THREAD
#warning using DETACHED_THREADS
	void RpcAppPair(pexeCtx_t pexe);
#else
	void RpcAppPair(pchMsg_t pmsg);
#endif

	void RpcAppExit(pexeCtx_t pexe);

	void CommandExecutor(pexeCtx_t pexe);

	void ProcessCommand(pchMsg_t & pmsg);


	/**
	 * @brief The command dispatching thread.
	 */
	void Dispatcher();

};

} // namespace bbque

#endif /* end of include guard: BBQUE_APPLICATION_PROXY_H_ */
