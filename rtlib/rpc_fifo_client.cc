/**
 *       @file  rpc_fifo.cc
 *      @brief  A message passing based RPC framework based on UNIX FIFO
 *
 * Definition of the RPC protocol based on UNIX FIFOs to implement the
 * Barbeque communication channel. This defines the communication protocol in
 * terms of message format and functionalities.
 * The communication protocol must be aligend with the RTLib supported
 * services.
 *
 * @see bbque/rtlib.h
 * @see bbque/rtlib/rpc_messages.h
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/13/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "bbque/rtlib/rpc_fifo_client.h"

#include "bbque/rtlib/rpc_messages.h"
#include "bbque/utils/utility.h"

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>


#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "RTLIB_FIFO [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "RTLIB_FIFO [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "RTLIB_FIFO [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "RTLIB_FIFO [ERR]", fmt)


#define RPC_FIFO_SEND(RPC_MSG)\
DB(fprintf(stderr, FMT_DBG("Tx [" #RPC_MSG "] Request "\
				"FIFO_HDR [sze: %hd, off: %hd, typ: %hd], "\
				"RPC_HDR [typ: %d, pid: %d, eid: %hd]...\n"),\
	rf_ ## RPC_MSG.hdr.fifo_msg_size,\
	rf_ ## RPC_MSG.hdr.rpc_msg_offset,\
	rf_ ## RPC_MSG.hdr.rpc_msg_type,\
	rf_ ## RPC_MSG.pyl.hdr.typ,\
	rf_ ## RPC_MSG.pyl.hdr.app_pid,\
	rf_ ## RPC_MSG.pyl.hdr.exc_id\
));\
if(::write(server_fifo_fd, (void*)&rf_ ## RPC_MSG, FIFO_PKT_SIZE(RPC_MSG)) <= 0) {\
	fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s]\n"),\
		bbque_fifo_path.c_str());\
	return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;\
}


namespace bbque { namespace rtlib {

BbqueRPC_FIFO_Client::BbqueRPC_FIFO_Client() :
	BbqueRPC(),
	app_fifo_path(BBQUE_PUBLIC_FIFO_PATH"/"),
	bbque_fifo_path(BBQUE_PUBLIC_FIFO_PATH"/"BBQUE_PUBLIC_FIFO) {

	DB(fprintf(stderr, FMT_DBG("Building FIFO RPC channel\n")));
}

BbqueRPC_FIFO_Client::~BbqueRPC_FIFO_Client() {
	DB(fprintf(stderr, FMT_DBG("BbqueRPC_FIFO_Client dtor\n")));
	ChannelRelease();
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::ChannelRelease() {
	rpc_fifo_APP_EXIT_t rf_APP_EXIT = {
		{
			FIFO_PKT_SIZE(APP_EXIT),
			FIFO_PYL_OFFSET(APP_EXIT),
			RPC_APP_EXIT
		},
		{
			{
				RPC_APP_EXIT,
				RpcMsgToken(),
				chTrdPid,
				0
			}
		}
	};
	int error;

	DB(fprintf(stderr, FMT_DBG("Releasing FIFO RPC channel\n")));

	// Sending RPC Request
	RPC_FIFO_SEND(APP_EXIT);

	// Closing the private FIFO
	error = ::unlink(app_fifo_path.c_str());
	if (error) {
		fprintf(stderr, FMT_ERR("FAILED unlinking the application FIFO [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(), errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_TEARDOWN_FAILED;
	}
	return RTLIB_OK;

}

void BbqueRPC_FIFO_Client::RpcBbqResp() {
	size_t bytes;

	// Read response RPC header
	bytes = ::read(client_fifo_fd, (void*)&chResp, RPC_PKT_SIZE(resp));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		chResp.result = RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}

	// Notify about reception of a new response
	DB(fprintf(stderr, FMT_INF("Notify response [%d]\n"), chResp.result));
	chResp_cv.notify_one();
}

void BbqueRPC_FIFO_Client::RpcBbqCmd_SetWorkingMode() {
	size_t bytes;

	// Read response RPC header
	bytes = ::read(client_fifo_fd, (void*)&chResp, RPC_PKT_SIZE(resp));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		chResp.result = RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}

	// Parse the assigned working mode

	// Notify about reception of a new response
	DB(fprintf(stderr, FMT_INF("Notify SWM [%d]\n"), chResp.result));
	chResp_cv.notify_one();
}

void BbqueRPC_FIFO_Client::ChannelFetch() {
	rpc_fifo_header_t hdr;
	size_t bytes;

	DB(fprintf(stderr, FMT_INF("Waiting for FIFO header...\n")));

	// Read FIFO header
	bytes = ::read(client_fifo_fd, (void*)&hdr, FIFO_PKT_SIZE(header));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		assert(bytes==FIFO_PKT_SIZE(header));
		// Exit the read thread if we are unable to read from the Barbeque
		// FIXME an error should be notified to the application
		done = true;
		return;
	}

	// Dispatching the received message
	switch (hdr.rpc_msg_type) {

	//--- Application Originated Messages
	case RPC_APP_RESP:
		DB(fprintf(stderr, FMT_INF("APP_RESP\n")));
		RpcBbqResp();
		break;

	//--- Execution Context Originated Messages
	case RPC_EXC_RESP:
		DB(fprintf(stderr, FMT_INF("EXC_RESP\n")));
		RpcBbqResp();
		break;

	//--- Barbeque Originated Messages
	case RPC_BBQ_STOP_EXECUTION:
		DB(fprintf(stderr, FMT_INF("BBQ_STOP_EXECUTION\n")));
		break;
	case RPC_BBQ_SYNCP_PRECHANGE:
		DB(fprintf(stderr, FMT_INF("BBQ_SYNCP_PRECHANGE\n")));
		RpcBbqSyncpPrechange();
		break;
	case RPC_BBQ_SYNCP_SYNCCHANGE:
		DB(fprintf(stderr, FMT_INF("BBQ_SYNCP_SYNCCHANGE\n")));
		//RpcBbqSyncpPostcYhange();
		break;
	case RPC_BBQ_SYNCP_DOCHANGE:
		DB(fprintf(stderr, FMT_INF("BBQ_SYNCP_DOCHANGE\n")));
		//RpcBbqSyncpDochange();
		break;
	case RPC_BBQ_SYNCP_POSTCHANGE:
		DB(fprintf(stderr, FMT_INF("BBQ_SYNCP_POSTCHANGE\n")));
		//RpcBbqSyncpPostchange();
		break;

	default:
		fprintf(stderr, FMT_ERR("Unknown BBQ response/command [%d]\n"),
				hdr.rpc_msg_type);
		assert(false);
		break;
	}
}

void BbqueRPC_FIFO_Client::ChannelTrd() {
	std::unique_lock<std::mutex> chSetup_ul(chSetup_mtx);
	// Getting client PID
	chTrdPid = gettid();
	DB(fprintf(stderr, FMT_INF("channel thread [PID: %d] started\n"),
				chTrdPid));
	// Notifying the thread has beed started
	trdStarted_cv.notify_one();

	// Waiting for channel setup to be completed
	chSetup_cv.wait(chSetup_ul);

	while (!done)
		ChannelFetch();
}


RTLIB_ExitCode BbqueRPC_FIFO_Client::ChannelPair(const char *name) {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	rpc_fifo_APP_PAIR_t rf_APP_PAIR = {
		{
			FIFO_PKT_SIZE(APP_PAIR),
			FIFO_PYL_OFFSET(APP_PAIR),
			RPC_APP_PAIR
		},
		"\0",
		{
			{
				RPC_APP_PAIR,
				RpcMsgToken(),
				chTrdPid,
				0
			},
			BBQUE_RPC_FIFO_MAJOR_VERSION,
			BBQUE_RPC_FIFO_MINOR_VERSION,
			"\0"
		}
	};
	::strncpy(rf_APP_PAIR.rpc_fifo, app_fifo_filename, BBQUE_FIFO_NAME_LENGTH);
	::strncpy(rf_APP_PAIR.pyl.app_name, name, RTLIB_APP_NAME_LENGTH);

	DB(fprintf(stderr, FMT_DBG("Pairing FIFO channels [app: %s, pid: %d]\n"),
					name, chTrdPid));

	// Sending RPC Request
	RPC_FIFO_SEND(APP_PAIR);

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	chResp_cv.wait_for(chCommand_ul, std::chrono::milliseconds(500));
	return (RTLIB_ExitCode)chResp.result;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::ChannelSetup() {
	RTLIB_ExitCode result = RTLIB_OK;
	int error;

	DB(fprintf(stderr, FMT_INF("Initializing channel\n")));

	// Opening server FIFO
	DB(fprintf(stderr, FMT_DBG("Opening bbque fifo [%s]...\n"),
				bbque_fifo_path.c_str()));
	server_fifo_fd = ::open(bbque_fifo_path.c_str(), O_WRONLY|O_NONBLOCK);
	if (server_fifo_fd < 0) {
		fprintf(stderr, FMT_ERR("FAILED opening bbque fifo [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(), errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
	}

	// Setting up application FIFO complete path
	app_fifo_path += app_fifo_filename;

	DB(fprintf(stderr, FMT_DBG("Creating [%s]...\n"),
				app_fifo_path.c_str()));

	// Creating the client side pipe
	error = ::mkfifo(app_fifo_path.c_str(), 0644);
	if (error) {
		fprintf(stderr, FMT_ERR("FAILED creating application FIFO [%s]\n"),
				app_fifo_path.c_str());
		result = RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
		goto err_create;
	}

	DB(fprintf(stderr, FMT_DBG("Opening R/W...\n")));

	// Opening the client side pipe
	// NOTE: this is opened R/W to keep it opened even if server
	// should disconnect
	client_fifo_fd = ::open(app_fifo_path.c_str(), O_RDWR);
	if (client_fifo_fd < 0) {
		fprintf(stderr, FMT_ERR("FAILED opening application FIFO [%s]\n"),
				app_fifo_path.c_str());
		result = RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
		goto err_open;
	}

	return RTLIB_OK;

err_open:
	::unlink(app_fifo_path.c_str());
err_create:
	::close(server_fifo_fd);
	return result;

}



RTLIB_ExitCode BbqueRPC_FIFO_Client::_Init(
			const char *name) {
	std::unique_lock<std::mutex> trdStatus_ul(trdStatus_mtx);
	RTLIB_ExitCode result;

	// Starting the communication thread
	done = false;
	ChTrd = std::thread(&BbqueRPC_FIFO_Client::ChannelTrd, this);
	trdStarted_cv.wait(trdStatus_ul);

	// Setting up application FIFO filename
	snprintf(app_fifo_filename, BBQUE_FIFO_NAME_LENGTH,
			"bbque_%05d_%s", chTrdPid, name);

	// Setting up the communication channel
	result = ChannelSetup();
	if (result != RTLIB_OK)
		return result;

	// Start the reception thread
	chSetup_cv.notify_one();

	// Pairing channel with server
	result = ChannelPair(name);
	if (result != RTLIB_OK) {
		::unlink(app_fifo_path.c_str());
		::close(server_fifo_fd);
		return result;
	}

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Register(pregExCtx_t prec) {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	rpc_fifo_EXC_REGISTER_t rf_EXC_REGISTER = {
		{
			FIFO_PKT_SIZE(EXC_REGISTER),
			FIFO_PYL_OFFSET(EXC_REGISTER),
			RPC_EXC_REGISTER
		},
		{
			{
				RPC_EXC_REGISTER,
				RpcMsgToken(),
				chTrdPid,
				prec->exc_id
			},
			"\0",
			"\0"
		}
	};
	::strncpy(rf_EXC_REGISTER.pyl.exc_name, prec->name.c_str(),
			RTLIB_EXC_NAME_LENGTH);
	::strncpy(rf_EXC_REGISTER.pyl.recipe, prec->exc_params.recipe,
			RTLIB_EXC_NAME_LENGTH);

	DB(fprintf(stderr, FMT_DBG("Registering EXC [%d:%d:%s]...\n"),
				rf_EXC_REGISTER.pyl.hdr.app_pid,
				rf_EXC_REGISTER.pyl.hdr.exc_id,
				rf_EXC_REGISTER.pyl.exc_name));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_REGISTER);

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	chResp_cv.wait_for(chCommand_ul, std::chrono::milliseconds(500));
	return (RTLIB_ExitCode)chResp.result;

}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Unregister(pregExCtx_t prec) {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	rpc_fifo_EXC_UNREGISTER_t rf_EXC_UNREGISTER = {
		{
			FIFO_PKT_SIZE(EXC_UNREGISTER),
			FIFO_PYL_OFFSET(EXC_UNREGISTER),
			RPC_EXC_UNREGISTER
		},
		{
			{
				RPC_EXC_UNREGISTER,
				RpcMsgToken(),
				chTrdPid,
				prec->exc_id
			},
			"\0"
		}
	};
	::strncpy(rf_EXC_UNREGISTER.pyl.exc_name, prec->name.c_str(),
			RTLIB_EXC_NAME_LENGTH);

	DB(fprintf(stderr, FMT_DBG("Unregistering EXC [%d:%d:%s]...\n"),
				rf_EXC_UNREGISTER.pyl.hdr.app_pid,
				rf_EXC_UNREGISTER.pyl.hdr.exc_id,
				rf_EXC_UNREGISTER.pyl.exc_name));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_UNREGISTER);

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	chResp_cv.wait_for(chCommand_ul, std::chrono::milliseconds(500));
	return (RTLIB_ExitCode)chResp.result;

}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Start(pregExCtx_t prec) {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	rpc_fifo_EXC_START_t rf_EXC_START = {
		{
			FIFO_PKT_SIZE(EXC_START),
			FIFO_PYL_OFFSET(EXC_START),
			RPC_EXC_START
		},
		{
			{
				RPC_EXC_START,
				RpcMsgToken(),
				chTrdPid,
				prec->exc_id
			},
		}
	};

	DB(fprintf(stderr, FMT_DBG("Starting EXC [%d:%d]...\n"),
				rf_EXC_START.pyl.hdr.app_pid,
				rf_EXC_START.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_START);

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	chResp_cv.wait_for(chCommand_ul, std::chrono::milliseconds(500));
	return (RTLIB_ExitCode)chResp.result;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Stop(pregExCtx_t prec) {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	rpc_fifo_EXC_STOP_t rf_EXC_STOP = {
		{
			FIFO_PKT_SIZE(EXC_STOP),
			FIFO_PYL_OFFSET(EXC_STOP),
			RPC_EXC_STOP
		},
		{
			{
				RPC_EXC_STOP,
				RpcMsgToken(),
				chTrdPid,
				prec->exc_id
			},
		}
	};

	DB(fprintf(stderr, FMT_DBG("Stopping EXC [%d:%d]...\n"),
				rf_EXC_STOP.pyl.hdr.app_pid,
				rf_EXC_STOP.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_STOP);

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	chResp_cv.wait_for(chCommand_ul, std::chrono::milliseconds(500));
	return (RTLIB_ExitCode)chResp.result;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Set(
			const RTLIB_ExecutionContextHandler ech,
			RTLIB_Constraint* constraints,
			uint8_t count) {
	//Silence "args not used" warning.
	(void)ech;
	(void)constraints;
	(void)count;

	fprintf(stderr, FMT_WRN("EXC Set: not yet implemeted\n"));

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Clear(
			const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;

	fprintf(stderr, FMT_WRN("EXC Clear: not yet implemeted\n"));

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_ScheduleRequest(pregExCtx_t prec) {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	rpc_fifo_EXC_SCHEDULE_t rf_EXC_SCHEDULE = {
		{
			FIFO_PKT_SIZE(EXC_SCHEDULE),
			FIFO_PYL_OFFSET(EXC_SCHEDULE),
			RPC_EXC_SCHEDULE
		},
		{
			{
				RPC_EXC_SCHEDULE,
				RpcMsgToken(),
				chTrdPid,
				prec->exc_id
			},
		}
	};

	DB(fprintf(stderr, FMT_DBG("Schedule request for EXC [%d:%d]...\n"),
				rf_EXC_SCHEDULE.pyl.hdr.app_pid,
				rf_EXC_SCHEDULE.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_SCHEDULE);

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	chResp_cv.wait_for(chCommand_ul, std::chrono::milliseconds(500));
	return (RTLIB_ExitCode)chResp.result;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_SyncpPrechangeResp(
		rpc_msg_token_t token, pregExCtx_t prec, uint32_t syncLatency) {

	rpc_fifo_BBQ_SYNCP_PRECHANGE_RESP_t rf_BBQ_SYNCP_PRECHANGE_RESP = {
		{
			FIFO_PKT_SIZE(BBQ_SYNCP_PRECHANGE_RESP),
			FIFO_PYL_OFFSET(BBQ_SYNCP_PRECHANGE_RESP),
			RPC_BBQ_RESP
		},
		{
			{
				RPC_BBQ_RESP,
				token,
				chTrdPid,
				prec->exc_id
			},
			syncLatency,
			RTLIB_OK
		}
	};

	DB(fprintf(stderr, FMT_DBG("PreChange response EXC [%d:%d] "
					"latency [%d]...\n"),
				rf_BBQ_SYNCP_PRECHANGE_RESP.pyl.hdr.app_pid,
				rf_BBQ_SYNCP_PRECHANGE_RESP.pyl.hdr.exc_id,
				rf_BBQ_SYNCP_PRECHANGE_RESP.pyl.syncLatency));

	// Sending RPC Request
	RPC_FIFO_SEND(BBQ_SYNCP_PRECHANGE_RESP);

	return RTLIB_OK;
}

void BbqueRPC_FIFO_Client::RpcBbqSyncpPrechange() {
	rpc_msg_BBQ_SYNCP_PRECHANGE_t msg;
	size_t bytes;

	// Read response RPC header
	bytes = ::read(client_fifo_fd, (void*)&msg,
			RPC_PKT_SIZE(BBQ_SYNCP_PRECHANGE));
	if (bytes <= 0) {
		fprintf(stderr, FMT_ERR("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		chResp.result = RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}

	// Notify the Pre-Change
	SyncP_PreChangeNotify(msg.hdr.token, msg.hdr.exc_id);

}

void BbqueRPC_FIFO_Client::_Exit() {
	ChannelRelease();
}

} // namespace rtlib

} // namespace bbque

