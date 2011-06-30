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
#include "bbque/resource_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/app/working_mode.h"
#include "bbque/utils/utility.h"

namespace ba = bbque::app;

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

	logger->Debug("APRs PRX: RX [typ: %d, pid: %d]",
			pChMsg->typ, pChMsg->app_pid);

	return (rpc_msg_type_t)pChMsg->typ;
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
	rpc_msg_BBQ_STOP_t stop_msg = {
		// FIXME The token should be defined as the thread id
		{RPC_BBQ_STOP_EXECUTION, 1234, papp->Pid(), papp->ExcId()},
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
	rpc->SendMessage(pcon->pd, &stop_msg.hdr,
			(size_t)RPC_PKT_SIZE(BBQ_STOP));

	return RTLIB_OK;
}

void ApplicationProxy::StopExecutionTrd(pcmdSn_t pcs) {
	(void)pcs;
#if 0
	pcmdRsp_t pcmdRsp(new cmdRsp_t);

	// Enqueuing the Command Session Handler
	EnqueueHandler(pcs);

	logger->Debug("APPs PRX [%5d]: StopExecutionTrd(%s) START",
			pcs->pid, pcs->papp->StrId());

	// Run the Command Synchronously
	pcmdRsp->result = StopExecutionSync(pcs->papp);
	// Give back the result to the calling thread
	(pcs->resp_prm).set_value(pcmdRsp);

	logger->Debug("APPs PRX [%5d]: StopExecutionTrd(%s) END",
			pcs->pid, pcs->papp->StrId());
#endif
}

RTLIB_ExitCode
ApplicationProxy::StopExecution(AppPtr_t papp) {
	(void)papp;
#if 0
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
#endif
	return RTLIB_OK;
}

/*******************************************************************************
 * Synchronization Protocol - PreChange
 ******************************************************************************/

RTLIB_ExitCode
ApplicationProxy::SyncP_PreChangeSend(pcmdSn_t pcs) {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx,
			std::defer_lock);
	conCtxMap_t::iterator it;
	AppPtr_t papp = pcs->papp;
	pconCtx_t pcon;
	rpc_msg_BBQ_SYNCP_PRECHANGE_t syncp_prechange_msg = {
		{RPC_BBQ_SYNCP_PRECHANGE, pcs->pid, papp->Pid(), papp->ExcId()},
		(uint8_t)papp->SyncState(),
		papp->NextAWM()->Id()
	};

	// Send the required synchronization action
	logger->Debug("APPs PRX: Send Command [RPC_BBQ_SYNCP_PRECHANGE] to "
			"EXC [%s], Action [%d:%s]", papp->StrId(), papp->SyncState(),
			ApplicationStatusIF::SyncStateStr(papp->SyncState()));

	assert(rpc);

	// Recover the communication context for this application
	conCtxMap_ul.lock();
	it = conCtxMap.find(papp->Pid());
	conCtxMap_ul.unlock();

	// Check we have a connection context already configured
	if (it == conCtxMap.end()) {
		logger->Error("APPs PRX: Send Command [RPC_BBQ_SYNCP_PRECHANGE] "
				"to EXC [%s] FAILED (Error: connection context not found)",
				papp->StrId());
		return RTLIB_BBQUE_CHANNEL_UNAVAILABLE;
	}

	// Sending message on the application connection context
	pcon = (*it).second;
	rpc->SendMessage(pcon->pd, &syncp_prechange_msg.hdr,
			(size_t)RPC_PKT_SIZE(BBQ_SYNCP_PRECHANGE));

	return RTLIB_OK;
}

RTLIB_ExitCode
ApplicationProxy::SyncP_PreChangeRecv(pcmdSn_t pcs,
		pPreChangeRsp_t &presp) {
	std::unique_lock<std::mutex> resp_ul(pcs->resp_mtx);
	rpc_msg_BBQ_SYNCP_PRECHANGE_RESP_t *pmsg_pyl;
	rpc_msg_header_t *pmsg_hdr;
	pchMsg_t pchMsg;

	// Wait for a response being available
	(pcs->resp_cv).wait(resp_ul);

	// Getting command response
	pchMsg = pcs->pmsg;
	pmsg_hdr = pchMsg;
	pmsg_pyl = (rpc_msg_BBQ_SYNCP_PRECHANGE_RESP_t*)pmsg_hdr;

	logger->Debug("APPs PRX: command response [typ: %d, pid: %d]",
			pmsg_hdr->typ,
			pmsg_hdr->app_pid);

	assert(pmsg_hdr->typ == RPC_BBQ_RESP);

	// Build a new communication context
	logger->Debug("APPs PRX: PreChangeResp [pid: %d, latency: %u]",
			pmsg_hdr->app_pid, pmsg_pyl->syncLatency);

	// Processing response
	presp->syncLatency = pmsg_pyl->syncLatency;

	return RTLIB_OK;
}

RTLIB_ExitCode
ApplicationProxy::SyncP_PreChange(pcmdSn_t pcs, pPreChangeRsp_t &presp) {

	// Send the Command
	presp->result = SyncP_PreChangeSend(pcs);
	if (presp->result != RTLIB_OK)
		return presp->result;

	// Get back the response
	presp->result = SyncP_PreChangeRecv(pcs, presp);
	if (presp->result != RTLIB_OK)
		return presp->result;

	return RTLIB_OK;

}

void
ApplicationProxy::SyncP_PreChangeTrd(pPreChangeRsp_t &presp) {

	// Enqueuing the Command Session Handler
	EnqueueHandler(presp->pcs);

	logger->Debug("APPs PRX [%5d]: SyncP_PreChangeTrd(%s) START",
			presp->pcs->pid, presp->pcs->papp->StrId());

	// Run the Command Executor
	SyncP_PreChange(presp->pcs, presp);

	// Give back the result to the calling thread
	(presp->pcs->resp_prm).set_value(presp->result);

	logger->Debug("APPs PRX [%5d]: SyncP_PreChangeTrd(%s) END",
			presp->pcs->pid, presp->pcs->papp->StrId());
}

RTLIB_ExitCode
ApplicationProxy::SyncP_PreChange(AppPtr_t papp, pPreChangeRsp_t &presp) {
	presp->pcs = SetupCmdSession(papp);

	// Enqueuing the Command Session Handler
	EnqueueHandler(presp->pcs);

	// Run the Command Executor
	return SyncP_PreChange(presp->pcs, presp);
}

RTLIB_ExitCode
ApplicationProxy::SyncP_PreChange_Async(AppPtr_t papp, pPreChangeRsp_t &presp) {
	presp->pcs = SetupCmdSession(papp);

	// Spawn a new Command Executor (passing the future)
	presp->pcs->exe = std::thread(&ApplicationProxy::SyncP_PreChangeTrd,
			this, std::ref(presp));

	// Setup the promise (thus unlocking the executor)
	presp->pcs->resp_ftr = (presp->pcs->resp_prm).get_future();

	return RTLIB_OK;
}

RTLIB_ExitCode
ApplicationProxy::SyncP_PreChange_GetResult(pPreChangeRsp_t &presp) {
	// Wait for the promise being returned
	presp->pcs->resp_ftr.wait();
	return presp->pcs->resp_ftr.get();
}

/*******************************************************************************
 * Synchronization Protocol - SyncChange
 ******************************************************************************/

RTLIB_ExitCode
ApplicationProxy::SyncP_SyncChangeSend(pcmdSn_t pcs) {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx,
			std::defer_lock);
	conCtxMap_t::iterator it;
	AppPtr_t papp = pcs->papp;
	pconCtx_t pcon;
	rpc_msg_BBQ_SYNCP_SYNCCHANGE_t syncp_syncchange_msg = {
		{RPC_BBQ_SYNCP_SYNCCHANGE, pcs->pid, papp->Pid(), papp->ExcId()}
	};

	// Send the stop command
	logger->Debug("APPs PRX: Send Command [RPC_BBQ_SYNCP_SYNCCHANGE] to "
			"EXC [%s]", papp->StrId());

	assert(rpc);

	// Recover the communication context for this application
	conCtxMap_ul.lock();
	it = conCtxMap.find(papp->Pid());
	conCtxMap_ul.unlock();

	// Check we have a connection context already configured
	if (it == conCtxMap.end()) {
		logger->Error("APPs PRX: Send Command [RPC_BBQ_SYNCP_SYNCCHANGE] "
				"to EXC [%s] FAILED (Error: connection context not found)",
				papp->StrId());
		return RTLIB_BBQUE_CHANNEL_UNAVAILABLE;
	}

	// Sending message on the application connection context
	pcon = (*it).second;
	rpc->SendMessage(pcon->pd, &syncp_syncchange_msg.hdr,
			(size_t)RPC_PKT_SIZE(BBQ_SYNCP_SYNCCHANGE));

	return RTLIB_OK;
}

RTLIB_ExitCode
ApplicationProxy::SyncP_SyncChangeRecv(pcmdSn_t pcs,
		pSyncChangeRsp_t &presp) {
	std::unique_lock<std::mutex> resp_ul(pcs->resp_mtx);
	rpc_msg_BBQ_SYNCP_SYNCCHANGE_RESP_t *pmsg_pyl;
	rpc_msg_header_t *pmsg_hdr;
	pchMsg_t pchMsg;

	// Wait for a response being available
	(pcs->resp_cv).wait(resp_ul);

	// Getting command response
	pchMsg = pcs->pmsg;
	pmsg_hdr = pchMsg;
	pmsg_pyl = (rpc_msg_BBQ_SYNCP_SYNCCHANGE_RESP_t*)pmsg_hdr;

	logger->Debug("APPs PRX: command response [typ: %d, pid: %d]",
			pmsg_hdr->typ,
			pmsg_hdr->app_pid);

	assert(pmsg_hdr->typ == RPC_BBQ_RESP);

	// Build a new communication context
	logger->Debug("APPs PRX: SyncChangeResp [pid: %d]", pmsg_hdr->app_pid);

	// Processing response (nothing to process right now)
	(void)presp;

	return RTLIB_OK;
}

RTLIB_ExitCode
ApplicationProxy::SyncP_SyncChange(pcmdSn_t pcs, pSyncChangeRsp_t &presp) {

	// Send the Command
	presp->result = SyncP_SyncChangeSend(pcs);
	if (presp->result != RTLIB_OK)
		return presp->result;

	// Get back the response
	presp->result = SyncP_SyncChangeRecv(pcs, presp);
	if (presp->result != RTLIB_OK)
		return presp->result;

	return RTLIB_OK;

}

void
ApplicationProxy::SyncP_SyncChangeTrd(pSyncChangeRsp_t &presp) {

	// Enqueuing the Command Session Handler
	EnqueueHandler(presp->pcs);

	logger->Debug("APPs PRX [%5d]: SyncP_SyncChangeTrd(%s) START",
			presp->pcs->pid, presp->pcs->papp->StrId());

	// Run the Command Executor
	SyncP_SyncChange(presp->pcs, presp);

	// Give back the result to the calling thread
	(presp->pcs->resp_prm).set_value(presp->result);

	logger->Debug("APPs PRX [%5d]: SyncP_SyncChangeTrd(%s) END",
			presp->pcs->pid, presp->pcs->papp->StrId());
}

RTLIB_ExitCode
ApplicationProxy::SyncP_SyncChange(AppPtr_t papp, pSyncChangeRsp_t &presp) {
	presp->pcs = SetupCmdSession(papp);

	// Enqueuing the Command Session Handler
	EnqueueHandler(presp->pcs);

	// Run the Command Executor
	return SyncP_SyncChange(presp->pcs, presp);
}

RTLIB_ExitCode
ApplicationProxy::SyncP_SyncChange_Async(AppPtr_t papp, pSyncChangeRsp_t &presp) {
	//ApplicationProxy::pcmdSn_t pcs(SetupCmdSession(papp));
	presp->pcs = SetupCmdSession(papp);

	// Spawn a new Command Executor (passing the future)
	presp->pcs->exe = std::thread(&ApplicationProxy::SyncP_SyncChangeTrd,
			this, std::ref(presp));

	// Setup the promise (thus unlocking the executor)
	presp->pcs->resp_ftr = (presp->pcs->resp_prm).get_future();

	return RTLIB_OK;
}

RTLIB_ExitCode
ApplicationProxy::SyncP_SyncChange_GetResult(pSyncChangeRsp_t &presp) {
	// Wait for the promise being returned
	presp->pcs->resp_ftr.wait();
	return presp->pcs->resp_ftr.get();
}


/*******************************************************************************
 * Synchronization Protocol - DoChange
 ******************************************************************************/

RTLIB_ExitCode
ApplicationProxy::SyncP_DoChangeSend(pcmdSn_t pcs) {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx,
			std::defer_lock);
	conCtxMap_t::iterator it;
	AppPtr_t papp = pcs->papp;
	pconCtx_t pcon;
	rpc_msg_BBQ_SYNCP_DOCHANGE_t syncp_syncchange_msg = {
		{RPC_BBQ_SYNCP_DOCHANGE, pcs->pid, papp->Pid(), papp->ExcId()}
	};

	// Send the stop command
	logger->Debug("APPs PRX: Send Command [RPC_BBQ_SYNCP_DOCHANGE] to "
			"EXC [%s]", papp->StrId());

	assert(rpc);

	// Recover the communication context for this application
	conCtxMap_ul.lock();
	it = conCtxMap.find(papp->Pid());
	conCtxMap_ul.unlock();

	// Check we have a connection context already configured
	if (it == conCtxMap.end()) {
		logger->Error("APPs PRX: Send Command [RPC_BBQ_SYNCP_DOCHANGE] "
				"to EXC [%s] FAILED (Error: connection context not found)",
				papp->StrId());
		return RTLIB_BBQUE_CHANNEL_UNAVAILABLE;
	}

	// Sending message on the application connection context
	pcon = (*it).second;
	rpc->SendMessage(pcon->pd, &syncp_syncchange_msg.hdr,
			(size_t)RPC_PKT_SIZE(BBQ_SYNCP_DOCHANGE));

	return RTLIB_OK;
}

RTLIB_ExitCode
ApplicationProxy::SyncP_DoChange(pcmdSn_t pcs, pDoChangeRsp_t &presp) {

	// Send the Command
	presp->result = SyncP_DoChangeSend(pcs);
	if (presp->result != RTLIB_OK)
		return presp->result;

	return RTLIB_OK;

}

RTLIB_ExitCode
ApplicationProxy::SyncP_DoChange(AppPtr_t papp) {
	pDoChangeRsp_t presp = ApplicationProxy::pDoChangeRsp_t(
			new ApplicationProxy::doChangeRsp_t());

	presp->pcs = SetupCmdSession(papp);

	// Run the Command Executor
	return SyncP_DoChange(presp->pcs, presp);
}


/*******************************************************************************
 * Synchronization Protocol - PostChange
 ******************************************************************************/

RTLIB_ExitCode
ApplicationProxy::SyncP_PostChangeSend(pcmdSn_t pcs) {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx,
			std::defer_lock);
	conCtxMap_t::iterator it;
	AppPtr_t papp = pcs->papp;
	pconCtx_t pcon;
	rpc_msg_BBQ_SYNCP_POSTCHANGE_t syncp_syncchange_msg = {
		{RPC_BBQ_SYNCP_POSTCHANGE, pcs->pid, papp->Pid(), papp->ExcId()}
	};

	// Send the stop command
	logger->Debug("APPs PRX: Send Command [RPC_BBQ_SYNCP_POSTCHANGE] to "
			"EXC [%s]", papp->StrId());

	assert(rpc);

	// Recover the communication context for this application
	conCtxMap_ul.lock();
	it = conCtxMap.find(papp->Pid());
	conCtxMap_ul.unlock();

	// Check we have a connection context already configured
	if (it == conCtxMap.end()) {
		logger->Error("APPs PRX: Send Command [RPC_BBQ_SYNCP_POSTCHANGE] "
				"to EXC [%s] FAILED (Error: connection context not found)",
				papp->StrId());
		return RTLIB_BBQUE_CHANNEL_UNAVAILABLE;
	}

	// Sending message on the application connection context
	pcon = (*it).second;
	rpc->SendMessage(pcon->pd, &syncp_syncchange_msg.hdr,
			(size_t)RPC_PKT_SIZE(BBQ_SYNCP_POSTCHANGE));

	return RTLIB_OK;
}

RTLIB_ExitCode
ApplicationProxy::SyncP_PostChangeRecv(pcmdSn_t pcs,
		pPostChangeRsp_t &presp) {
	std::unique_lock<std::mutex> resp_ul(pcs->resp_mtx);
	rpc_msg_BBQ_SYNCP_POSTCHANGE_RESP_t *pmsg_pyl;
	rpc_msg_header_t *pmsg_hdr;
	pchMsg_t pchMsg;

	// Wait for a response being available
	(pcs->resp_cv).wait(resp_ul);

	// Getting command response
	pchMsg = pcs->pmsg;
	pmsg_hdr = pchMsg;
	pmsg_pyl = (rpc_msg_BBQ_SYNCP_POSTCHANGE_RESP_t*)pmsg_hdr;

	logger->Debug("APPs PRX: command response [typ: %d, pid: %d]",
			pmsg_hdr->typ,
			pmsg_hdr->app_pid);

	assert(pmsg_hdr->typ == RPC_BBQ_RESP);

	// Build a new communication context
	logger->Debug("APPs PRX: PostChangeResp [pid: %d]", pmsg_hdr->app_pid);

	// Processing response (nothing to process right now)
	(void)presp;

	return RTLIB_OK;
}

RTLIB_ExitCode
ApplicationProxy::SyncP_PostChange(pcmdSn_t pcs, pPostChangeRsp_t &presp) {

	// Send the Command
	presp->result = SyncP_PostChangeSend(pcs);
	if (presp->result != RTLIB_OK)
		return presp->result;

	// Get back the response
	presp->result = SyncP_PostChangeRecv(pcs, presp);
	if (presp->result != RTLIB_OK)
		return presp->result;

	return RTLIB_OK;

}

RTLIB_ExitCode
ApplicationProxy::SyncP_PostChange(AppPtr_t papp, pPostChangeRsp_t &presp) {
	presp->pcs = SetupCmdSession(papp);

	// Enqueuing the Command Session Handler
	EnqueueHandler(presp->pcs);

	// Run the Command Executor
	return SyncP_PostChange(presp->pcs, presp);
}



/*******************************************************************************
 * Command Sessions Helpers
 ******************************************************************************/

ApplicationProxy::pcmdSn_t
ApplicationProxy::GetCommandSession(rpc_msg_header_t *pmsg_hdr)  {
	std::unique_lock<std::mutex> cmdSnMap_ul(cmdSnMap_mtx);
	cmdSnMap_t::iterator it;
	pcmdSn_t pcs;

	// Looking for a valid command session
	it = cmdSnMap.find(pmsg_hdr->token);
	if (it == cmdSnMap.end()) {
		cmdSnMap_ul.unlock();
		logger->Warn("APPs PRX [%5d]: Command transation completion FAILED",
			"(Error: command session not found)", pmsg_hdr->token);
		assert(it != cmdSnMap.end());
		return pcmdSn_t();
	}
	return (*it).second;

}

void ApplicationProxy::CompleteTransaction(pchMsg_t & pmsg) {
	rpc_msg_header_t * pmsg_hdr = pmsg;
	pcmdSn_t pcs;

	assert(pmsg_hdr);

	logger->Debug("APPs PRX: dispatching command response "
			"[typ: %d, pid: %d] to [%5d]...",
			pmsg_hdr->typ,
			pmsg_hdr->app_pid,
			pmsg_hdr->token);

	// Looking for a valid command session
	pcs = GetCommandSession(pmsg_hdr);
	if (!pcs) {
		logger->Crit("APPs PRX: dispatching command response FAILED "
				"(Error: cmd session not found for token [%d])",
				pmsg_hdr->token);
		assert(pcs);
		return;
	}

	// Setup command session response buffer
	pcs->pmsg = pmsg;

	// Notify command session
	//std::unique_lock<std::mutex> resp_ul(pcs->resp_mtx);
	(pcs->resp_cv).notify_one();

}


/*******************************************************************************
 * Request Sessions
 ******************************************************************************/

#define APP_STRID "[%05d:%6s]"
#define AppStrId(pcon) pcon->app_pid, pcon->app_name 

#define EXC_STRID "[%05d:%6s:%02d]"
#define ExcStrId(pcon, exc_id) pcon->app_pid, pcon->app_name, exc_id  

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
	logger->Debug("APPs PRX: Send RPC channel ACK "APP_STRID, AppStrId(pcon));
	::memcpy(&resp.hdr, pmsg_hdr, RPC_PKT_SIZE(header));
	resp.hdr.typ = type;
	resp.result = RTLIB_OK;
	rpc->SendMessage(pcon->pd, &resp.hdr, (size_t)RPC_PKT_SIZE(resp));

}

void ApplicationProxy::RpcNAK(pconCtx_t pcon, rpc_msg_header_t *pmsg_hdr,
		rpc_msg_type_t type, RTLIB_ExitCode error) {
	rpc_msg_resp_t resp;

	// Sending response to application
	logger->Debug("APPs PRX: Send RPC channel NAK "APP_STRID", error [%d]",
			AppStrId(pcon), error);
	::memcpy(&resp.hdr, pmsg_hdr, RPC_PKT_SIZE(header));
	resp.hdr.typ = type;
	resp.result = error;
	rpc->SendMessage(pcon->pd, &resp.hdr, (size_t)RPC_PKT_SIZE(resp));

}

void ApplicationProxy::RpcExcRegister(prqsSn_t prqs) {
	std::unique_lock<std::mutex> conCtxMap_ul(conCtxMap_mtx, std::defer_lock);
	ApplicationManager &am(ApplicationManager::GetInstance());
	pchMsg_t pchMsg = prqs->pmsg;
	rpc_msg_header_t * pmsg_hdr = pchMsg;
	rpc_msg_EXC_REGISTER_t * pmsg_pyl = (rpc_msg_EXC_REGISTER_t*)pmsg_hdr;
	pconCtx_t pcon;
	AppPtr_t papp;

	assert(pchMsg);

	// Looking for a valid connection context
	pcon = GetConnectionContext(pmsg_hdr);
	if (!pcon)
		return;

	// Registering a new Execution Context
	logger->Info("APPs PRX: Registering EXC "EXC_STRID", name [%s]",
			ExcStrId(pcon, pmsg_hdr->exc_id), pmsg_pyl->exc_name);

	// Register the EXC with the ApplicationManager
	papp  = am.CreateEXC(pmsg_pyl->exc_name, pcon->app_pid,
			pmsg_hdr->exc_id, pmsg_pyl->recipe);
	if (!papp) {
		logger->Error("APPs PRX: EXC "EXC_STRID", name [%s] "
			"registration FAILED "
			"(Error: missing recipe or recipe load failure)",
			ExcStrId(pcon, pmsg_hdr->exc_id), pmsg_pyl->exc_name);
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
	rpc_msg_EXC_UNREGISTER_t * pmsg_pyl =
		(rpc_msg_EXC_UNREGISTER_t*)pmsg_hdr;
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

void ApplicationProxy::RpcExcSchedule(prqsSn_t prqs) {
	ResourceManager &rm(ResourceManager::GetInstance());
	rpc_msg_header_t * pmsg_hdr = prqs->pmsg;
	pconCtx_t pcon;

	assert(pmsg_hdr);

	// Looking for a valid connection context
	pcon = GetConnectionContext(pmsg_hdr);
	if (!pcon)
		return;

	// Registering a new Execution Context
	logger->Info("APPs PRX: Schedule request for EXC "
			"[app: %s, pid: %d, exc: %d]",
			pcon->app_name, pcon->app_pid, pmsg_hdr->exc_id);

	// Notify the ResourceManager for a new application willing to start
	logger->Debug("APPs PRX: Notifing ResourceManager...");
	rm.NotifyEvent(ResourceManager::EXC_START);

	// Sending ACK response to application
	RpcACK(pcon, pmsg_hdr, RPC_EXC_RESP);

}

void ApplicationProxy::RpcAppPair(prqsSn_t prqs) {
	std::unique_lock<std::mutex> conCtxMap_ul(
			conCtxMap_mtx, std::defer_lock);
	pchMsg_t pchMsg = prqs->pmsg;
	rpc_msg_header_t * pmsg_hdr = pchMsg;
	rpc_msg_APP_PAIR_t * pmsg_pyl = (rpc_msg_APP_PAIR_t*)pmsg_hdr;
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

	case RPC_EXC_SCHEDULE:
		logger->Debug("EXC_SCHEDULE");
		RpcExcSchedule(prqs);
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
	snCtxMap.insert(std::pair<uint8_t, psnCtx_t>(
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

