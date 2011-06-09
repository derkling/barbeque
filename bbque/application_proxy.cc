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

#include "bbque/application_manager.h"
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


/*******************************************************************************
 * Command Sessions
 ******************************************************************************/

inline ApplicationProxy::pcmdSn_t ApplicationProxy::SetupCmdSession(
		AppPtr_t papp) const {
	pcmdSn_t psn = pcmdSn_t(new cmdSn_t);
	psn->papp = papp;
	psn->resp_prm = resp_prm_t();
	return psn;
}

inline void ApplicationProxy::EnqueueHandler(pcmdSn_t pcs) {
	std::unique_lock<std::mutex> cmdSnMap_ul(cmdSnMap_mtx);
	pcs->pid = gettid();

	if (cmdSnMap.find(pcs->pid) != cmdSnMap.end()) {
		logger->Crit("APPs PRX: handler enqueuing FAILED "
				"(Error: possible data structure corruption)");
		assert(cmdSnMap.find(pcs->pid)==cmdSnMap.end());
		return;
	}

	cmdSnMap.insert(std::pair<pid_t, pcmdSn_t>(pcs->pid, pcs));
}

RTLIB_ExitCode ApplicationProxy::StopExecutionSync(AppPtr_t papp) {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx,
			std::defer_lock);
	conCtxMap_t::iterator it;
	pconCtx_t pcon;
	rpc_msg_bbq_stop_t stop_msg = {
		{RPC_BBQ_STOP_EXECUTION, papp->Pid(), papp->ExcId()},
		{0, 100} // FIXME get a timeout parameter
	};

	// Send the stop command
	logger->Debug("APPs PRX: Send Command [RPC_BBQ_STOP_EXECUTION] to "
			"[app: %s, pid: %d, exc: %d]",
			papp->Name().c_str(), papp->Pid(), papp->ExcId());

	assert(rpc);

	// Recover the communication context for this application
	conCtxMap_ul.lock();
	it = conCtxMap.find(papp->Pid());
	conCtxMap_ul.unlock();

	// Check we have a connection contexe already configured
	if (it == conCtxMap.end()) {
		logger->Error("APPs PRX: Connection context not found "
				"for application "
				"[app: %s, pid: %d]",
				papp->Name().c_str(), papp->Pid());
		return RTLIB_BBQUE_CHANNEL_UNAVAILABLE;
	}

	// Sending message on the application connection context
	pcon = (*it).second;
	rpc->SendMessage(pcon->pd, &stop_msg.header,
			(size_t)RPC_PKT_SIZE(bbq_stop));

	return RTLIB_OK;
}

void ApplicationProxy::StopExecutionTrd(pcmdSn_t snHdr) {
	pcmdRsp_t pcmdRsp(new cmdRsp_t);

	// Enqueuing the Command Session Handler
	EnqueueHandler(snHdr);

	logger->Debug("APPs PRX: StopExecutionTrd [pid: %5d] "
			"START [app: %s, pid: %d, exc: %d]",
			snHdr->pid, snHdr->papp->Name().c_str(),
			snHdr->papp->Pid(), snHdr->papp->ExcId());

	// Run the Command Synchronously
	pcmdRsp->result = StopExecutionSync(snHdr->papp);
	// Give back the result to the calling thread
	(snHdr->resp_prm).set_value(pcmdRsp);

	logger->Debug("APPs PRX: StopExecutionTrd [pid: %5d] "
			"END [app: %s, pid: %d, exc: %d]",
			snHdr->pid, snHdr->papp->Name().c_str(),
			snHdr->papp->Pid(), snHdr->papp->ExcId());

}

ApplicationProxy::resp_ftr_t ApplicationProxy::StopExecution(AppPtr_t papp) {
	ApplicationProxy::pcmdSn_t psn;

	// Ensure the application is still active
	assert(papp->State() < Application::FINISHED);
	if (papp->State() >= Application::FINISHED) {
		logger->Warn("Multiple stopping the same application [%s]",
				papp->Name().c_str());
		return resp_ftr_t();
	}

	// Setup a new command session
	psn = SetupCmdSession(papp);

	// Spawn a new Command Executor (passing the future)
	psn->exe = std::thread(&ApplicationProxy::StopExecutionTrd, this, psn);

	// Return the promise (thus unlocking the executor)
	return resp_ftr_t((psn->resp_prm).get_future());

}


void ApplicationProxy::CompleteTransaction(pchMsg_t & msg) {
	logger->Debug("APPs PRX: processing transaction response...");
	(void)msg;
}


/*******************************************************************************
 * Request Sessions
 ******************************************************************************/
ApplicationProxy::pconCtx_t ApplicationProxy::GetConnectionContext(
		rpc_msg_header_t *pmsg_hdr)  {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx);
	conCtxMap_t::iterator it;
	pconCtx_t pcon;

	// Looking for a valid connection context
	it = conCtxMap.find(pmsg_hdr->app_pid);
	if (it == conCtxMap.end()) {
		conCtxMap_ul.unlock();
		logger->Warn("APPs PRX: EXC registration FAILED",
			"[pid: %d, exc: %d] (Error: application not paired)",
			pmsg_hdr->app_pid, pmsg_hdr->exc_id);
		return pconCtx_t();
	}
	return (*it).second;
}

void ApplicationProxy::RpcACK(pconCtx_t pcon, rpc_msg_header_t *pmsg_hdr,
		rpc_msg_type_t type) {
	rpc_msg_resp_t resp;

	// Sending response to application
	logger->Debug("APPs PRX: Send RPC channel ACK [pid: %d, name: %s]",
			pcon->app_pid, pcon->app_name);
	::memcpy(&resp.header, pmsg_hdr, RPC_PKT_SIZE(header));
	resp.header.typ = type;
	resp.result = RTLIB_OK;
	rpc->SendMessage(pcon->pd, &resp.header, (size_t)RPC_PKT_SIZE(resp));

}

void ApplicationProxy::RpcNAK(pconCtx_t pcon, rpc_msg_header_t *pmsg_hdr,
		rpc_msg_type_t type, RTLIB_ExitCode error) {
	rpc_msg_resp_t resp;

	// Sending response to application
	logger->Debug("APPs PRX: Send RPC channel NAK "
			"[pid: %d, name: %s, err: %d]",
			pcon->app_pid, pcon->app_name, error);
	::memcpy(&resp.header, pmsg_hdr, RPC_PKT_SIZE(header));
	resp.header.typ = type;
	resp.result = error;
	rpc->SendMessage(pcon->pd, &resp.header, (size_t)RPC_PKT_SIZE(resp));

}

void ApplicationProxy::RpcExcRegister(prqsSn_t prqs) {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx, std::defer_lock);
	ApplicationManager &am(ApplicationManager::GetInstance());
	pchMsg_t pchMsg = prqs->pmsg;
	rpc_msg_header_t * pmsg_hdr = pchMsg;
	rpc_msg_exc_register_t * pmsg_pyl = (rpc_msg_exc_register_t*)pmsg_hdr;
	pconCtx_t pcon;
	AppPtr_t papp;

	assert(pchMsg);

	// Looking for a valid connection context
	pcon = GetConnectionContext(pmsg_hdr);
	if (!pcon)
		return;

	// Registering a new Execution Context
	logger->Info("APPs PRX: Registering EXC "
			"[app: %s, pid: %d, exc: %d, nme: %s]",
			pcon->app_name, pcon->app_pid,
			pmsg_hdr->exc_id, pmsg_pyl->exc_name);

	// Register the EXC with the ApplicationManager
	papp  = am.CreateEXC(pmsg_pyl->exc_name, pcon->app_pid,
			pmsg_hdr->exc_id, pmsg_pyl->recipe);
	if (!papp) {
		logger->Error("APPs PRX: EXC "
			"[app: %s, pid: %d, exc: %d, nme: %s] "
			"registration FAILED "
			"(Error: missing recipe or recipe load failure)",
			pcon->app_name, pcon->app_pid,
			pmsg_hdr->exc_id, pmsg_pyl->exc_name);
		RpcNAK(pcon, pmsg_hdr, RPC_EXC_RESP, RTLIB_EXC_MISSING_RECIPE);
		return;
	}

	// Sending ACK response to application
	RpcACK(pcon, pmsg_hdr, RPC_EXC_RESP);

}

void ApplicationProxy::RpcExcUnregister(prqsSn_t prqs) {
	ApplicationManager &am(ApplicationManager::GetInstance());
	pchMsg_t pchMsg = prqs->pmsg;
	rpc_msg_header_t * pmsg_hdr = pchMsg;
	rpc_msg_exc_unregister_t * pmsg_pyl =
		(rpc_msg_exc_unregister_t*)pmsg_hdr;
	pconCtx_t pcon;

	assert(pchMsg);

	// Looking for a valid connection context
	pcon = GetConnectionContext(pmsg_hdr);
	if (!pcon)
		return;

	// Unregister an Execution Context
	logger->Info("APPs PRX: Unregistering EXC "
			"[app: %s, pid: %d, exc: %d, nme: %s]",
			pcon->app_name, pcon->app_pid,
			pmsg_hdr->exc_id, pmsg_pyl->exc_name);

	// Unregister the EXC from the ApplicationManager
	am.DestroyEXC(pcon->app_pid, pmsg_hdr->exc_id);
	// FIXME we should deliver the error code to the application
	// This is tracked by Issues tiket #10

	// Sending ACK response to application
	RpcACK(pcon, pmsg_hdr, RPC_EXC_RESP);

}

void ApplicationProxy::RpcExcStart(prqsSn_t prqs) {
	ApplicationManager &am(ApplicationManager::GetInstance());
	pchMsg_t pchMsg = prqs->pmsg;
	rpc_msg_header_t * pmsg_hdr = pchMsg;
	pconCtx_t pcon;
	ApplicationManager::ExitCode_t result;

	assert(pchMsg);

	// Looking for a valid connection context
	pcon = GetConnectionContext(pmsg_hdr);
	if (!pcon)
		return;

	// Registering a new Execution Context
	logger->Info("APPs PRX: Starting EXC "
			"[app: %s, pid: %d, exc: %d]",
			pcon->app_name, pcon->app_pid, pmsg_hdr->exc_id);

	// Enabling the EXC to the ApplicationManager
	result = am.EnableEXC(pcon->app_pid, pmsg_hdr->exc_id);
	if (result != ApplicationManager::AM_SUCCESS) {
		logger->Error("APPs PRX: EXC "
			"[pid: %d, exc: %d] "
			"start FAILED",
			pcon->app_pid, pmsg_hdr->exc_id);
		RpcNAK(pcon, pmsg_hdr, RPC_EXC_RESP, RTLIB_EXC_START_FAILED);
		return;
	}

	// Sending ACK response to application
	RpcACK(pcon, pmsg_hdr, RPC_EXC_RESP);

}

void ApplicationProxy::RpcExcStop(prqsSn_t prqs) {
	ApplicationManager &am(ApplicationManager::GetInstance());
	pchMsg_t pchMsg = prqs->pmsg;
	rpc_msg_header_t * pmsg_hdr = pchMsg;
	pconCtx_t pcon;
	ApplicationManager::ExitCode_t result;

	assert(pchMsg);

	// Looking for a valid connection context
	pcon = GetConnectionContext(pmsg_hdr);
	if (!pcon)
		return;

	// Registering a new Execution Context
	logger->Info("APPs PRX: Stopping EXC "
			"[app: %s, pid: %d, exc: %d]",
			pcon->app_name, pcon->app_pid, pmsg_hdr->exc_id);

	// Enabling the EXC to the ApplicationManager
	result = am.DisableEXC(pcon->app_pid, pmsg_hdr->exc_id);
	if (result != ApplicationManager::AM_SUCCESS) {
		logger->Error("APPs PRX: EXC "
			"[pid: %d, exc: %d] "
			"stop FAILED",
			pcon->app_pid, pmsg_hdr->exc_id);
		RpcNAK(pcon, pmsg_hdr, RPC_EXC_RESP, RTLIB_EXC_STOP_FAILED);
		return;
	}

	// Sending ACK response to application
	RpcACK(pcon, pmsg_hdr, RPC_EXC_RESP);

}

void ApplicationProxy::RpcExcGwm(prqsSn_t prqs) {
	pchMsg_t pchMsg = prqs->pmsg;
	rpc_msg_header_t * pmsg_hdr = pchMsg;
	pconCtx_t pcon;

	assert(pchMsg);

	// Looking for a valid connection context
	pcon = GetConnectionContext(pmsg_hdr);
	if (!pcon)
		return;

	// Registering a new Execution Context
	logger->Info("APPs PRX: GetWorkingMode for EXC "
			"[app: %s, pid: %d, exc: %d]",
			pcon->app_name, pcon->app_pid, pmsg_hdr->exc_id);

	// Running the resource allocator if needed
	// This return success if a scheduling has been started, false if EXC
	// is not enabled or the scheduled could not be run.
	logger->Warn("APPs PRX: TODO run optimizer");

	// Sending ACK response to application
	RpcACK(pcon, pmsg_hdr, RPC_EXC_RESP);

}

void ApplicationProxy::RpcAppPair(prqsSn_t prqs) {
	std::unique_lock<std::mutex> conCtxMap_ul(
			conCtxMap_mtx, std::defer_lock);
	pchMsg_t pchMsg = prqs->pmsg;
	rpc_msg_header_t * pmsg_hdr = pchMsg;
	rpc_msg_app_pair_t * pmsg_pyl = (rpc_msg_app_pair_t*)pmsg_hdr;
	pconCtx_t pcon;

	// Ensure this application has not yet registerd
	assert(pmsg_hdr->typ == RPC_APP_PAIR);
	assert(conCtxMap.find(pmsg_hdr->app_pid) == conCtxMap.end());

	// Build a new communication context
	logger->Debug("APPs PRX: Setting-up RPC channel [pid: %d, name: %s]...",
			pmsg_hdr->app_pid, pmsg_pyl->app_name);

	// Checking API versioning
	if (pmsg_pyl->mjr_version != RTLIB_VERSION_MAJOR ||
			pmsg_pyl->mnr_version > RTLIB_VERSION_MINOR) {
		logger->Error("APPs PRX: Setup RPC channel [pid: %d, name: %s] "
				"FAILED (Error: version mismatch, "
				"app_v%d.%d != rtlib_v%d.%d)",
			pmsg_hdr->app_pid, pmsg_pyl->app_name,
			pmsg_pyl->mjr_version, pmsg_pyl->mnr_version,
			RTLIB_VERSION_MAJOR, RTLIB_VERSION_MINOR);
		return;
	}

	// Setting up a new communication context
	pcon = pconCtx_t(new conCtx_t);
	assert(pcon);
	if (!pcon) {
		logger->Error("APPs PRX: Setup RPC channel [pid: %d, name: %s] "
				"FAILED (Error: connection context setup)",
			pcon->app_pid,
			pcon->app_name);
		// TODO If an application do not get a responce withih a timeout
		// should try again to register. This support should be provided
		// by the RTLib
		return;
	}

	pcon->app_pid = pmsg_hdr->app_pid;
	::strncpy(pcon->app_name, pmsg_pyl->app_name, RTLIB_APP_NAME_LENGTH);
	pcon->pd = rpc->GetPluginData(pchMsg);
	assert(pcon->pd);
	if (!pcon->pd) {
		logger->Error("APPs PRX: Setup RPC channel [pid: %d, name: %s] "
				"FAILED (Error: communication channel setup)",
			pcon->app_pid,
			pcon->app_name);
		// TODO If an application do not get a responce withih a timeout
		// should try again to register. This support should be provided
		// by the RTLib
		return;
	}

	// Backup communication context for further messages
	conCtxMap_ul.lock();
	conCtxMap.insert(std::pair<pid_t, pconCtx_t>(
				pcon->app_pid, pcon));
	conCtxMap_ul.unlock();

	// Sending ACK response to application
	RpcACK(pcon, pmsg_hdr, RPC_APP_RESP);
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
	logger->Info("APPs PRX: Application [app_pid: %d] ended, "
			"releasing resources...",
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

void ApplicationProxy::RequestExecutor(prqsSn_t prqs) {
	std::unique_lock<std::mutex> snCtxMap_ul(snCtxMap_mtx);
	snCtxMap_t::iterator it;
	psnCtx_t psc;

	// Set the thread PID
	prqs->pid = gettid();

	// This look could be acquiren only when the ProcessCommand has returned
	// Thus we use this lock to synchronize the CommandExecutr starting with
	// the completion of the ProcessCommand, which also setup thread
	// tracking data structures.
	snCtxMap_ul.unlock();

	logger->Debug("APPs PRX [%d:%d]: RequestExecutor START",
			prqs->pid, prqs->pmsg->typ);

	assert(prqs->pmsg->typ<RPC_EXC_MSGS_COUNT);

	// TODO put here command execution code
	switch(prqs->pmsg->typ) {
	case RPC_EXC_REGISTER:
		logger->Debug("EXC_REGISTER");
		RpcExcRegister(prqs);
		break;

	case RPC_EXC_UNREGISTER:
		logger->Debug("EXC_UNREGISTER");
		RpcExcUnregister(prqs);
		break;

	case RPC_EXC_SET:
		logger->Debug("EXC_SET");
		break;

	case RPC_EXC_CLEAR:
		logger->Debug("EXC_CLEAR");
		break;

	case RPC_EXC_START:
		logger->Debug("EXC_START");
		RpcExcStart(prqs);
		break;

	case RPC_EXC_STOP:
		logger->Debug("EXC_STOP");
		RpcExcStop(prqs);
		break;

	case RPC_EXC_GWM:
		logger->Debug("EXC_GWM");
		RpcExcGwm(prqs);
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

	logger->Debug("APPs PRX [%d:%d]: RequestExecutor END",
			prqs->pid, prqs->pmsg->typ);

}

void ApplicationProxy::ProcessRequest(pchMsg_t & pmsg) {
	std::unique_lock<std::mutex> snCtxMap_ul(snCtxMap_mtx);
	prqsSn_t prqsSn = prqsSn_t(new rqsSn_t);
	assert(prqsSn);

	prqsSn->pmsg = pmsg;
	// Create a new executor thread, this will start locked since it needs
	// the execMap_mtx we already hold. This is used to ensure that the
	// executor thread start only alfter the playground has been properly
	// prepared
	prqsSn->exe = std::thread(
				&ApplicationProxy::RequestExecutor,
				this, prqsSn);
	prqsSn->exe.detach();

	logger->Debug("APPs PRX: Processing NEW REQUEST...");

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

	logger->Info("APPs PRX: Messages dispatcher STARTED");

	while(1) {
		if (GetNextMessage(pmsg)>RPC_EXC_MSGS_COUNT) {
			CompleteTransaction(pmsg);
			continue;
		}
		ProcessRequest(pmsg);
	}

	logger->Info("APPs PRX: Messages dispatcher ENDED");
}

} // namespace bbque

