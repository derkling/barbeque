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

#include "bbque/app/application.h"
#include "bbque/plugins/logger.h"
#include "bbque/plugins/rpc_channel.h"
#include "bbque/rtlib/rpc_messages.h"

#include <future>
#include <memory>
#include <thread>

#include <map>

using namespace bbque::plugins;
using namespace bbque::rtlib;
using namespace bbque::app;

namespace bbque {

class ApplicationProxy {

private:

	std::thread dispatcher_thd;

	plugins::LoggerIF *logger;

	plugins::RPCChannelIF *rpc;


	typedef struct snCtx {
		std::thread exe;
		AppPid_t pid;
	} snCtx_t;

	typedef std::promise<RTLIB_ExitCode> resp_prm_t;

	typedef std::future<RTLIB_ExitCode> resp_ftr_t;

	typedef std::shared_ptr<resp_ftr_t> prespFtr_t;

	typedef RPCChannelIF::rpc_msg_ptr_t pchMsg_t;

	typedef struct cmdSn : public snCtx_t {
		AppPtr_t papp;
		resp_prm_t resp_prm;
		resp_ftr_t resp_ftr;
		std::mutex resp_mtx;
		std::condition_variable resp_cv;
		pchMsg_t pmsg;
	} cmdSn_t;

	typedef std::shared_ptr<cmdSn_t> pcmdSn_t;

	typedef struct cmdRsp {
		RTLIB_ExitCode result;
		// The comand session to handler this command
		pcmdSn_t pcs;
	} cmdRsp_t;




public:

	static ApplicationProxy & GetInstance();

	void Start();


	~ApplicationProxy();

/*******************************************************************************
 * Command Sessions
 ******************************************************************************/

	RTLIB_ExitCode StopExecution(AppPtr_t papp);

	RTLIB_ExitCode StopExecutionSync(AppPtr_t papp);

/*******************************************************************************
 * Synchronization Protocol
 ******************************************************************************/

	typedef struct preChangeRsp : public cmdRsp_t {
		uint32_t syncLatency; ///> [ms] estimation of next sync point
	} preChangeRsp_t;

	typedef std::shared_ptr<preChangeRsp_t> pPreChangeRsp_t;

	/**
	 * @brief Synchronous PreChange
	 */
	RTLIB_ExitCode SyncP_PreChange(AppPtr_t papp, pPreChangeRsp_t &presp);

	/**
	 * @brief Start an Asynchronous PreChange
	 */
	RTLIB_ExitCode SyncP_PreChange_Async(AppPtr_t papp, pPreChangeRsp_t &presp);

	/**
	 * @brief Get the result of an issued Asynchronous PreChange
	 */
	RTLIB_ExitCode SyncP_PreChange_GetResult(pPreChangeRsp_t &presp);

private:



	typedef std::shared_ptr<snCtx_t> psnCtx_t;

	typedef std::map<rpc_msg_type_t, psnCtx_t> snCtxMap_t;

	snCtxMap_t snCtxMap;

	std::mutex snCtxMap_mtx;


	std::mutex trdStatus_mtx;

	std::condition_variable trdStatus_cv;

	typedef struct conCtx {
		/** The applicaiton PID */
		AppPid_t app_pid;
		/** The application name */
		char app_name[RTLIB_APP_NAME_LENGTH];
		/** The communication channel data to connect the applicaton */
		RPCChannelIF::plugin_data_t pd;
	} conCtx_t;

	typedef std::shared_ptr<conCtx_t> pconCtx_t;

	typedef std::map<AppPid_t, pconCtx_t> conCtxMap_t;

	conCtxMap_t conCtxMap;

	std::mutex conCtxMap_mtx;

	typedef struct rqsSn : public snCtx_t {
		pchMsg_t pmsg;
	} rqsSn_t;

	typedef std::shared_ptr<rqsSn_t> prqsSn_t;


	/**
	 * @brief	A multimap to track active Command Sessions.
	 *
	 * This multimap maps command session threads ID on the session data.
	 * @param AppPid_t the command session thread ID
	 * @param pcmdSn_t the command session handler
	 */
	typedef std::map<rpc_msg_token_t, pcmdSn_t> cmdSnMap_t;

	cmdSnMap_t cmdSnMap;

	std::mutex cmdSnMap_mtx;



	typedef std::shared_ptr<cmdRsp_t> pcmdRsp_t;


	ApplicationProxy();

	rpc_msg_type_t GetNextMessage(pchMsg_t & pmsg);


/*******************************************************************************
 * Command Sessions
 ******************************************************************************/

	inline pcmdSn_t SetupCmdSession(AppPtr_t papp) const;

	/**
	 * @brief Enqueue a command session for response processing
	 *
	 * Since Barbeque has a single input RPC channel for each application,
	 * each response received from an applications should be dispatched to the
	 * thread which generated the command. Thus, the execution context which
	 * has generated a command must save a reference to itself for the proper
	 * dispatching of resposes.
	 *
	 * @param pcs command session handler which is waiting for a response
	 *
	 * @note This method must be called from within the session execution
	 * context, i.e. the command processing thread for asynchronous commands.
	 */
	inline void EnqueueHandler(pcmdSn_t pcs);

	void StopExecutionTrd(pcmdSn_t pcs);

	pcmdSn_t GetCommandSession(rpc_msg_header_t *pmsg_hdr);

	void CompleteTransaction(pchMsg_t & pmsg);

/*******************************************************************************
 * Synchronization Protocol
 ******************************************************************************/

	RTLIB_ExitCode SyncP_PreChangeSend(pcmdSn_t pcs);

	RTLIB_ExitCode SyncP_PreChangeRecv(pcmdSn_t pcs, pPreChangeRsp_t &preps);

	RTLIB_ExitCode SyncP_PreChange(pcmdSn_t pcs, pPreChangeRsp_t &presp);

	void SyncP_PreChangeTrd(pPreChangeRsp_t &presp);

/*******************************************************************************
 * Request Sessions
 ******************************************************************************/

	void RpcExcRegister(prqsSn_t prqs);

	void RpcExcUnregister(prqsSn_t prqs);


	void RpcExcStart(prqsSn_t prqs);

	void RpcExcStop(prqsSn_t prqs);


	void RpcExcSchedule(prqsSn_t prqs);


	void RpcAppPair(prqsSn_t prqs);

	void RpcAppExit(prqsSn_t prqs);


	pconCtx_t GetConnectionContext(rpc_msg_header_t *pmsg_hdr);

	void RpcACK(pconCtx_t pcon, rpc_msg_header_t *pmsg_hdr,
			rpc_msg_type_t type);

	void RpcNAK(pconCtx_t pcon, rpc_msg_header_t * pmsg_hdr,
			rpc_msg_type_t type,
			RTLIB_ExitCode error);

	void RequestExecutor(prqsSn_t prqs);

	void ProcessRequest(pchMsg_t & pmsg);


	/**
	 * @brief The command dispatching thread.
	 */
	void Dispatcher();

};

} // namespace bbque

#endif /* end of include guard: BBQUE_APPLICATION_PROXY_H_ */
