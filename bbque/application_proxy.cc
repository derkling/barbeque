/**
 *       @file  application_proxy.cc
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

#include "bbque/application_proxy.h"

#include "bbque/modules_factory.h"
#include "bbque/utils/utility.h"

namespace bbque {

ApplicationProxy::ApplicationProxy() {

	//---------- Get a logger module
	std::string logger_name("bq.ap");
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

	//---------- Initialize the RPC channel module
	// TODO look the configuration file for the required channel
	// Build an RPCChannelIF object
	rpc = ModulesFactory::GetRPCChannelModule();
	if (!rpc) {
		logger->Fatal("RM: RPC Channel module creation FAILED");
		abort();
	}
	// RPC channel initialization
	if (rpc->Init()) {
		logger->Fatal("RM: RPC Channel module setup FAILED");
		abort();
	}

	// Spawn the command dispatching thread
	dispatcher_thd = std::thread(&ApplicationProxy::Dispatcher, this);

}

ApplicationProxy::~ApplicationProxy() {
	// TODO add code to release the RPC Channel module
}

ApplicationProxy & ApplicationProxy::GetInstance() {
	static ApplicationProxy instance;
	return instance;
}

void ApplicationProxy::Start() {
	std::unique_lock<std::mutex> ul(trdStatus_mtx);

	logger->Debug("AAPRs PRX: service starting...");
	trdStatus_cv.notify_one();
}

rpc_msg_type_t ApplicationProxy::GetNextMessage(pchMsg_t & pChMsg) {

	rpc->RecvMessage(pChMsg);

	logger->Debug("APRs PRX: rx [typ: %d, pid: %d]",
			pChMsg->typ, pChMsg->app_pid);

	return pChMsg->typ;
}


void ApplicationProxy::CompleteTransaction(pchMsg_t & msg) {
	logger->Debug("APPs PRX: processing transaction response...");
	(void)msg;
}


/*******************************************************************************
 * Request Sessions
 ******************************************************************************/

void ApplicationProxy::RpcAppPair(prqsSn_t prqs) {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx, std::defer_lock);
	pchMsg_t pmsg = prqs->pmsg;
	rpc_msg_resp_t resp;
	pconCtx_t pcon;

	// Ensure this application has not yet registerd
	assert(pmsg->typ == RPC_APP_PAIR);
	assert(conCtxMap.find(pmsg->app_pid) == conCtxMap.end());

	// Build a new communication context
	logger->Debug("APPs PRX: Setting-up RPC channel [app_pid: %d]...",
			pmsg->app_pid);
	pcon = pconCtx_t(new conCtx_t);
	pcon->app_pid = pmsg->app_pid;
	pcon->pd = rpc->GetPluginData(pmsg);

	if (!pcon->pd) {
		logger->Error("APPs PRX: Setup RPC channel [app_pid: %d] FAILED",
			pmsg->app_pid);
		// TODO If an application do not get a responce withih a timeout
		// should try again to register. This support should be provided by
		// the RTLib
		return;
	}

	// Backup communication context for further messages
	conCtxMap_ul.lock();
	conCtxMap.insert(std::pair<pid_t, pconCtx_t>(
				pmsg->app_pid, pcon));
	conCtxMap_ul.unlock();

	// Sending responce to application
	logger->Debug("APPs PRX: Send RPC channel ACK [app_pid: %d]",
			pmsg->app_pid);
	::memcpy(&resp.header, pmsg, RPC_PKT_SIZE(header));
	resp.header.typ = RPC_BBQ_RESP;
	resp.result = RTLIB_OK;
	rpc->SendMessage(pcon->pd, &resp.header, (size_t)RPC_PKT_SIZE(resp));

}

void ApplicationProxy::RpcAppExit(prqsSn_t prqs) {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx);
	pchMsg_t pmsg = prqs->pmsg;
	conCtxMap_t::iterator conCtxIt;
	pconCtx_t pconCtx;

	// Ensure this application is already registerd
	conCtxIt = conCtxMap.find(pmsg->app_pid);
	assert(conCtxIt!=conCtxMap.end());

	// Releasing application resources
	logger->Debug("APPs PRX: Application [app_pid: %d] ended, releasing resources...",
			pmsg->app_pid);

	// Cleanup communication channel resources
	pconCtx = (*conCtxIt).second;
	rpc->ReleasePluginData(pconCtx->pd);

	// Removing the connection context
	conCtxMap.erase(conCtxIt);
	conCtxMap_ul.unlock();

	logger->Warn("APPs PRX: TODO release all application resources");
	logger->Warn("APPs PRX: TODO run optimizer");

}

void ApplicationProxy::CommandExecutor(prqsSn_t prqs) {
	std::unique_lock<std::mutex> snCtxMap_ul(snCtxMap_mtx);
	snCtxMap_t::iterator it;
	psnCtx_t psc;

	// Set the thread PID
	prqs->pid = gettid();

	// This look could be acquiren only when the ProcessCommand has returned
	// Thus we use this lock to synchronize the CommandExecutr starting with
	// the completion of the ProcessCommand, which also setup thread tracking
	// data structures.
	snCtxMap_ul.unlock();

	logger->Debug("APPs PRX: CommandExecutor START [pid: %d, typ: %d]",
			prqs->pid, prqs->pmsg->typ);

	assert(prqs->pmsg->typ<RPC_APP_MSGS_COUNT);

	// TODO put here command execution code
	switch(prqs->pmsg->typ) {
	case RPC_EXC_REGISTER:
		logger->Debug("EXC_REGISTER");
		break;

	case RPC_EXC_UNREGISTER:
		logger->Debug("EXC_UNREGISTER");
		break;

	case RPC_EXC_SET:
		logger->Debug("EXC_SET");
		break;

	case RPC_EXC_CLEAR:
		logger->Debug("EXC_CLEAR");
		break;

	case RPC_EXC_START:
		logger->Debug("EXC_START");
		break;

	case RPC_EXC_STOP:
		logger->Debug("EXC_STOP");
		break;
	case RPC_APP_PAIR:
		logger->Debug("APP_PAIR");
		RpcAppPair(prqs);
		break;
	case RPC_APP_EXIT:
		logger->Debug("APP_EXIT");
		RpcAppExit(prqs);
		break;

	default:
		// Unknowen command
		assert(false);
		break;
	}

	// Releasing the thread tracking data before exiting
	snCtxMap_ul.lock();
	it = snCtxMap.lower_bound(prqs->pmsg->typ);
	for ( ; it != snCtxMap.end() ; it++) {
		psc = it->second;
		if (psc->pid == prqs->pid) {
			snCtxMap.erase(it);
			break;
		}
	}
	snCtxMap_ul.unlock();

	logger->Debug("APPs PRX: CommandExecutor END [pid: %d, typ: %d]",
			prqs->pid, prqs->pmsg->typ);

}

void ApplicationProxy::ProcessCommand(pchMsg_t & pmsg) {
	std::unique_lock<std::mutex> snCtxMap_ul(snCtxMap_mtx);
	prqsSn_t prqsSn = prqsSn_t(new rqsSn_t);
	prqsSn->pmsg = pmsg;
	// Create a new executor thread, this will start locked since it needs the
	// execMap_mtx we already hold. This is used to ensure that the executor
	// thread start only alfter the playground has been properly prepared
	prqsSn->exe = std::thread(
				&ApplicationProxy::CommandExecutor,
				this, prqsSn);
	prqsSn->exe.detach();

	logger->Info("Processing new command...");

	// Add a new threaded command executor
	snCtxMap.insert(std::pair<rpc_msg_type_t, psnCtx_t>(
				pmsg->typ, prqsSn));

}

void ApplicationProxy::Dispatcher() {
	std::unique_lock<std::mutex> trdStatus_ul(trdStatus_mtx);
	pchMsg_t pmsg;

	// Waiting for thread authorization to start
	trdStatus_cv.wait(trdStatus_ul);
	trdStatus_ul.unlock();

	logger->Info("Command dispatcher thread started");

	while(1) {
		if (GetNextMessage(pmsg)>RPC_APP_MSGS_COUNT) {
			CompleteTransaction(pmsg);
			continue;
		}
		ProcessCommand(pmsg);
	}

	logger->Info("Command dispatcher thread ended");
}

} // namespace bbque

