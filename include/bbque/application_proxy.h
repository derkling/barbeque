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

public:

	static ApplicationProxy & GetInstance();

	void Start();

	typedef struct cmdRsp {
		RTLIB_ExitCode result;
	} cmdRsp_t;

	typedef std::shared_ptr<cmdRsp_t> pcmdRsp_t;

	typedef std::promise<pcmdRsp_t> resp_prm_t;

	typedef std::future<pcmdRsp_t> resp_ftr_t;

	typedef std::shared_ptr<resp_ftr_t> prespFtr_t;

	resp_ftr_t StopExecution(AppPtr_t papp);

	RTLIB_ExitCode StopExecutionSync(AppPtr_t papp);

	~ApplicationProxy();

private:

	typedef RPCChannelIF::rpc_msg_ptr_t pchMsg_t;

	typedef struct snCtx {
		std::thread exe;
		AppPid_t pid;
	} snCtx_t;

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

	typedef struct cmdSn : public snCtx_t {
		AppPtr_t papp;
		resp_prm_t resp_prm;
		std::mutex resp_mtx;
		std::condition_variable resp_cv;
		pchMsg_t pmsg;
	} cmdSn_t;

	typedef std::shared_ptr<cmdSn_t> pcmdSn_t;


	/**
	 * @brief	A multimap to track active Command Sessions.
	 *
	 * This multimap maps command session threads ID on the session session
	 * data.
	 * @param AppPid_t the command session thread ID
	 * @param pcmdSn_t the command session handler
	 */
	typedef std::multimap<AppPid_t, pcmdSn_t> cmdSnMm_t;

	cmdSnMm_t cmdSnMm;

	std::mutex cmdSnMm_mtx;

	ApplicationProxy();

	rpc_msg_type_t GetNextMessage(pchMsg_t & pmsg);


/*******************************************************************************
 * Command Sessions
 ******************************************************************************/

	inline pcmdSn_t SetupCmdSession(AppPtr_t papp) const;

	inline void EnqueueHandler(pcmdSn_t snHdr);

	void StopExecutionTrd(pcmdSn_t snHdr);

	void CompleteTransaction(pchMsg_t & pmsg);

/*******************************************************************************
 * Request Sessions
 ******************************************************************************/

	void RpcExcRegister(prqsSn_t prqs);

	void RpcExcUnregister(prqsSn_t prqs);


	void RpcExcStart(prqsSn_t prqs);

	void RpcExcStop(prqsSn_t prqs);


	void RpcExcGwm(prqsSn_t prqs);


	void RpcAppPair(prqsSn_t prqs);

	void RpcAppExit(prqsSn_t prqs);


	pconCtx_t GetConnectionContext(rpc_msg_header_t *pmsg_hdr);

	void RpcExcACK(pconCtx_t pcon, rpc_msg_header_t *pmsg_hdr);

	void RpcExcNAK(pconCtx_t pcon, rpc_msg_header_t * pmsg_hdr,
			RTLIB_ExitCode error);

	void CommandExecutor(prqsSn_t prqs);

	void ProcessCommand(pchMsg_t & pmsg);


	/**
	 * @brief The command dispatching thread.
	 */
	void Dispatcher();

};

} // namespace bbque

#endif /* end of include guard: BBQUE_APPLICATION_PROXY_H_ */
