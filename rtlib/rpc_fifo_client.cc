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

#include "bbque/rtlib/rpc_fifo_client.h"

#include "bbque/rtlib/rpc_messages.h"
#include "bbque/utils/utility.h"
#include "bbque/config.h"

#include <sys/prctl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

// Setup logging
#undef  BBQUE_LOG_MODULE
#define BBQUE_LOG_MODULE "rpc.fif"
#undef  BBQUE_LOG_UID
#define BBQUE_LOG_UID GetUid()

#define RPC_FIFO_SEND_SIZE(RPC_MSG, SIZE)\
DB(fprintf(stderr, FD("Tx [" #RPC_MSG "] Request "\
				"FIFO_HDR [sze: %hd, off: %hd, typ: %hd], "\
				"RPC_HDR [typ: %d, pid: %d, eid: %"PRIu8"], Bytes: %lu...\n"),\
	rf_ ## RPC_MSG.hdr.fifo_msg_size,\
	rf_ ## RPC_MSG.hdr.rpc_msg_offset,\
	rf_ ## RPC_MSG.hdr.rpc_msg_type,\
	rf_ ## RPC_MSG.pyl.hdr.typ,\
	rf_ ## RPC_MSG.pyl.hdr.app_pid,\
	rf_ ## RPC_MSG.pyl.hdr.exc_id,\
	SIZE\
));\
if(::write(server_fifo_fd, (void*)&rf_ ## RPC_MSG, SIZE) <= 0) {\
	fprintf(stderr, FE("write to BBQUE fifo FAILED [%s]\n"),\
		bbque_fifo_path.c_str());\
	return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;\
}

#define RPC_FIFO_SEND(RPC_MSG)\
	RPC_FIFO_SEND_SIZE(RPC_MSG, FIFO_PKT_SIZE(RPC_MSG))


namespace bbque { namespace rtlib {

BbqueRPC_FIFO_Client::BbqueRPC_FIFO_Client() :
	BbqueRPC(),
	app_fifo_path(BBQUE_PATH_VAR"/"),
	bbque_fifo_path(BBQUE_PATH_VAR"/"BBQUE_PUBLIC_FIFO) {

	DB(fprintf(stderr, FD("Building FIFO RPC channel\n")));
}

BbqueRPC_FIFO_Client::~BbqueRPC_FIFO_Client() {
	DB(fprintf(stderr, FD("BbqueRPC_FIFO_Client dtor\n")));
	ChannelRelease();
}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::ChannelRelease() {
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

	DB(fprintf(stderr, FD("Releasing FIFO RPC channel\n")));

	// Sending RPC Request
	RPC_FIFO_SEND(APP_EXIT);

	// Sending the same message to the Fetch Thread
	if (::write(client_fifo_fd, (void*)&rf_APP_EXIT, FIFO_PKT_SIZE(APP_EXIT)) <= 0) {
		fprintf(stderr, FE("Notify fetch thread FAILED, FORCED EXIT\n"));
	} else {
		// Joining fetch thread
		ChTrd.join();
	}

	// Closing the private FIFO
	error = ::unlink(app_fifo_path.c_str());
	if (error) {
		fprintf(stderr, FE("FAILED unlinking the application FIFO [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(), errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_TEARDOWN_FAILED;
	}
	return RTLIB_OK;

}

void BbqueRPC_FIFO_Client::RpcBbqResp() {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	size_t bytes;

	// Read response RPC header
	bytes = ::read(client_fifo_fd, (void*)&chResp, RPC_PKT_SIZE(resp));
	if (bytes<=0) {
		fprintf(stderr, FE("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		chResp.result = RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}

	// Notify about reception of a new response
	DB(fprintf(stderr, FI("Notify response [%d]\n"), chResp.result));
	chResp_cv.notify_one();
}

void BbqueRPC_FIFO_Client::ChannelFetch() {
	rpc_fifo_header_t hdr;
	size_t bytes;

	DB(fprintf(stderr, FD("Waiting for FIFO header...\n")));

	// Read FIFO header
	bytes = ::read(client_fifo_fd, (void*)&hdr, FIFO_PKT_SIZE(header));
	if (bytes<=0) {
		fprintf(stderr, FE("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		assert(bytes==FIFO_PKT_SIZE(header));
		// Exit the read thread if we are unable to read from the Barbeque
		// FIXME an error should be notified to the application
		done = true;
		return;
	}

	DB(fprintf(stderr, FD("Rx FIFO_HDR "
					"[sze: %hd, off: %hd, typ: %hd]\n"),
				hdr.fifo_msg_size,
				hdr.rpc_msg_offset,
				hdr.rpc_msg_type));

	// Dispatching the received message
	switch (hdr.rpc_msg_type) {

	case RPC_APP_EXIT:
		done = true;
		break;

	//--- Application Originated Messages
	case RPC_APP_RESP:
		DB(fprintf(stderr, FI("APP_RESP\n")));
		RpcBbqResp();
		break;

	//--- Execution Context Originated Messages
	case RPC_EXC_RESP:
		DB(fprintf(stderr, FI("EXC_RESP\n")));
		RpcBbqResp();
		break;

	//--- Barbeque Originated Messages
	case RPC_BBQ_STOP_EXECUTION:
		DB(fprintf(stderr, FI("BBQ_STOP_EXECUTION\n")));
		break;
	case RPC_BBQ_SYNCP_PRECHANGE:
		DB(fprintf(stderr, FI("BBQ_SYNCP_PRECHANGE\n")));
		RpcBbqSyncpPreChange();
		break;
	case RPC_BBQ_SYNCP_SYNCCHANGE:
		DB(fprintf(stderr, FI("BBQ_SYNCP_SYNCCHANGE\n")));
		RpcBbqSyncpSyncChange();
		break;
	case RPC_BBQ_SYNCP_DOCHANGE:
		DB(fprintf(stderr, FI("BBQ_SYNCP_DOCHANGE\n")));
		RpcBbqSyncpDoChange();
		break;
	case RPC_BBQ_SYNCP_POSTCHANGE:
		DB(fprintf(stderr, FI("BBQ_SYNCP_POSTCHANGE\n")));
		RpcBbqSyncpPostChange();
		break;

	default:
		fprintf(stderr, FE("Unknown BBQ response/command [%d]\n"),
				hdr.rpc_msg_type);
		assert(false);
		break;
	}
}

void BbqueRPC_FIFO_Client::ChannelTrd(const char *name) {
	std::unique_lock<std::mutex> trdStatus_ul(trdStatus_mtx);

	// Set the thread name
	if (unlikely(prctl(PR_SET_NAME, (long unsigned int)"bq.fifo", 0, 0, 0)))
		fprintf(stderr, "Set name FAILED! (Error: %s)\n",
				strerror(errno));

	// Setup the RTLib UID
	setChId(gettid(), name);
	DB(fprintf(stderr, FI("channel thread [PID: %d] CREATED\n"),
				chTrdPid));
	// Notifying the thread has beed started
	trdStatus_cv.notify_one();

	// Waiting for channel setup to be completed
	if (!running)
		trdStatus_cv.wait(trdStatus_ul);

	DB(fprintf(stderr, FI("channel thread [PID: %d] START\n"),
				chTrdPid));
	while (!done)
		ChannelFetch();

	DB(fprintf(stderr, FI("channel thread [PID: %d] END\n"),
				chTrdPid));
}

#define WAIT_RPC_RESP \
	chResp.result = RTLIB_BBQUE_CHANNEL_TIMEOUT; \
	chResp_cv.wait_for(chCommand_ul, \
			std::chrono::milliseconds(BBQUE_RPC_TIMEOUT)); \
	if (chResp.result == RTLIB_BBQUE_CHANNEL_TIMEOUT) {\
		fprintf(stderr, FW("RTLIB response TIMEOUT\n")); \
	}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::ChannelPair(const char *name) {
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

	DB(fprintf(stderr, FD("Pairing FIFO channels [app: %s, pid: %d]\n"),
					name, chTrdPid));

	// Sending RPC Request
	RPC_FIFO_SEND(APP_PAIR);

	DB(fprintf(stderr, FD("Waiting BBQUE response...\n")));

	WAIT_RPC_RESP;
	return (RTLIB_ExitCode_t)chResp.result;
}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::ChannelSetup() {
	RTLIB_ExitCode_t result = RTLIB_OK;
	int error;

	DB(fprintf(stderr, FI("Initializing channel\n")));

	// Opening server FIFO
	DB(fprintf(stderr, FD("Opening bbque fifo [%s]...\n"),
				bbque_fifo_path.c_str()));
	server_fifo_fd = ::open(bbque_fifo_path.c_str(), O_WRONLY|O_NONBLOCK);
	if (server_fifo_fd < 0) {
		fprintf(stderr, FE("FAILED opening bbque fifo [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(), errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
	}

	// Setting up application FIFO complete path
	app_fifo_path += app_fifo_filename;

	DB(fprintf(stderr, FD("Creating [%s]...\n"),
				app_fifo_path.c_str()));

	// Creating the client side pipe
	error = ::mkfifo(app_fifo_path.c_str(), 0644);
	if (error) {
		fprintf(stderr, FE("FAILED creating application FIFO [%s]\n"),
				app_fifo_path.c_str());
		result = RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
		goto err_create;
	}

	DB(fprintf(stderr, FD("Opening R/W...\n")));

	// Opening the client side pipe
	// NOTE: this is opened R/W to keep it opened even if server
	// should disconnect
	client_fifo_fd = ::open(app_fifo_path.c_str(), O_RDWR);
	if (client_fifo_fd < 0) {
		fprintf(stderr, FE("FAILED opening application FIFO [%s]\n"),
				app_fifo_path.c_str());
		result = RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
		goto err_open;
	}

	// Ensuring the FIFO is R/W to everyone
	if (fchmod(client_fifo_fd, S_IRUSR|S_IWUSR|S_IWGRP|S_IWOTH)) {
		fprintf(stderr,
			FE("FAILED setting permissions on RPC FIFO [%s] "
				"(Error %d: %s)"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
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



RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_Init(
			const char *name) {
	std::unique_lock<std::mutex> trdStatus_ul(trdStatus_mtx);
	RTLIB_ExitCode_t result;

	// Starting the communication thread
	done = false;
	running = false;
	ChTrd = std::thread(&BbqueRPC_FIFO_Client::ChannelTrd, this, name);
	trdStatus_cv.wait(trdStatus_ul);

	// Setting up application FIFO filename
	snprintf(app_fifo_filename, BBQUE_FIFO_NAME_LENGTH,
			"bbque_%05d_%s", chTrdPid, name);

	// Setting up the communication channel
	result = ChannelSetup();
	if (result != RTLIB_OK)
		return result;

	// Start the reception thread
	running = true;
	trdStatus_cv.notify_one();
	trdStatus_ul.unlock();

	// Pairing channel with server
	result = ChannelPair(name);
	if (result != RTLIB_OK) {
		::unlink(app_fifo_path.c_str());
		::close(server_fifo_fd);
		return result;
	}

	return RTLIB_OK;
}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_Register(pregExCtx_t prec) {
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

	DB(fprintf(stderr, FD("Registering EXC [%d:%d:%s]...\n"),
				rf_EXC_REGISTER.pyl.hdr.app_pid,
				rf_EXC_REGISTER.pyl.hdr.exc_id,
				rf_EXC_REGISTER.pyl.exc_name));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_REGISTER);

	DB(fprintf(stderr, FD("Waiting BBQUE response...\n")));

	WAIT_RPC_RESP;
	return (RTLIB_ExitCode_t)chResp.result;

}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_Unregister(pregExCtx_t prec) {
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

	DB(fprintf(stderr, FD("Unregistering EXC [%d:%d:%s]...\n"),
				rf_EXC_UNREGISTER.pyl.hdr.app_pid,
				rf_EXC_UNREGISTER.pyl.hdr.exc_id,
				rf_EXC_UNREGISTER.pyl.exc_name));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_UNREGISTER);

	DB(fprintf(stderr, FD("Waiting BBQUE response...\n")));

	WAIT_RPC_RESP;
	return (RTLIB_ExitCode_t)chResp.result;

}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_Enable(pregExCtx_t prec) {
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

	DB(fprintf(stderr, FD("Enabling EXC [%d:%d]...\n"),
				rf_EXC_START.pyl.hdr.app_pid,
				rf_EXC_START.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_START);

	DB(fprintf(stderr, FD("Waiting BBQUE response...\n")));

	WAIT_RPC_RESP;
	return (RTLIB_ExitCode_t)chResp.result;
}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_Disable(pregExCtx_t prec) {
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

	DB(fprintf(stderr, FD("Disabling EXC [%d:%d]...\n"),
				rf_EXC_STOP.pyl.hdr.app_pid,
				rf_EXC_STOP.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_STOP);

	DB(fprintf(stderr, FD("Waiting BBQUE response...\n")));

	WAIT_RPC_RESP;
	return (RTLIB_ExitCode_t)chResp.result;
}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_Set(pregExCtx_t prec,
			RTLIB_Constraint_t* constraints, uint8_t count) {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	// Here the message is dynamically allocate to make room for a variable
	// number of constraints...
	rpc_fifo_EXC_SET_t *prf_EXC_SET;
	size_t msg_size;

	// At least 1 constraint it is expected
	assert(count);

	// Allocate the buffer to hold all the contraints
	msg_size = FIFO_PKT_SIZE(EXC_SET) + 
			((count-1)*sizeof(RTLIB_Constraint_t));
	prf_EXC_SET = (rpc_fifo_EXC_SET_t*)::malloc(msg_size);


	// Init FIFO header
	prf_EXC_SET->hdr.fifo_msg_size = msg_size;
	prf_EXC_SET->hdr.rpc_msg_offset = FIFO_PYL_OFFSET(EXC_SET);
	prf_EXC_SET->hdr.rpc_msg_type = RPC_EXC_SET;

	// Init RPC header
	prf_EXC_SET->pyl.hdr.typ = RPC_EXC_SET;
	prf_EXC_SET->pyl.hdr.token = RpcMsgToken();
	prf_EXC_SET->pyl.hdr.app_pid = chTrdPid;
	prf_EXC_SET->pyl.hdr.exc_id = prec->exc_id;

	DB(fprintf(stderr, FD("Copying [%d] constraints using buffer @%p "
					"of [%"PRIu64"] Bytes...\n"),
				count, (void*)&(prf_EXC_SET->pyl.constraints),
				(count)*sizeof(RTLIB_Constraint_t)));

	// Init RPC header
	prf_EXC_SET->pyl.count = count;
	::memcpy(&(prf_EXC_SET->pyl.constraints), constraints,
			(count)*sizeof(RTLIB_Constraint_t));

	// Sending RPC Request
	volatile rpc_fifo_EXC_SET_t & rf_EXC_SET = (*prf_EXC_SET);
	DB(fprintf(stderr, FD("Set [%d] constraints on EXC [%d:%d]...\n"),
				count, rf_EXC_SET.pyl.hdr.app_pid,
				rf_EXC_SET.pyl.hdr.exc_id));
	RPC_FIFO_SEND_SIZE(EXC_SET, msg_size);

	// Clean-up the FIFO message
	::free(prf_EXC_SET);

	DB(fprintf(stderr, FD("Waiting BBQUE response...\n")));

	WAIT_RPC_RESP;
	return (RTLIB_ExitCode_t)chResp.result;
}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_Clear(pregExCtx_t prec) {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	rpc_fifo_EXC_CLEAR_t rf_EXC_CLEAR = {
		{
			FIFO_PKT_SIZE(EXC_CLEAR),
			FIFO_PYL_OFFSET(EXC_CLEAR),
			RPC_EXC_CLEAR
		},
		{
			{
				RPC_EXC_CLEAR,
				RpcMsgToken(),
				chTrdPid,
				prec->exc_id
			},
		}
	};

	DB(fprintf(stderr, FD("Clear constraints for EXC [%d:%d]...\n"),
				rf_EXC_CLEAR.pyl.hdr.app_pid,
				rf_EXC_CLEAR.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_CLEAR);

	DB(fprintf(stderr, FD("Waiting BBQUE response...\n")));

	WAIT_RPC_RESP;
	return (RTLIB_ExitCode_t)chResp.result;
}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_GGap(pregExCtx_t prec, uint8_t gap) {
	std::unique_lock<std::mutex> chCommand_ul(chCommand_mtx);
	rpc_fifo_EXC_GGAP_t rf_EXC_GGAP = {
		{
			FIFO_PKT_SIZE(EXC_GGAP),
			FIFO_PYL_OFFSET(EXC_GGAP),
			RPC_EXC_GGAP
		},
		{
			{
				RPC_EXC_GGAP,
				RpcMsgToken(),
				chTrdPid,
				prec->exc_id
			},
			gap,
		}
	};

	DB(fprintf(stderr, FD("Set Goal-Gap for EXC [%d:%d]...\n"),
				rf_EXC_GGAP.pyl.hdr.app_pid,
				rf_EXC_GGAP.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_GGAP);

	DB(fprintf(stderr, FD("Waiting BBQUE response...\n")));

	WAIT_RPC_RESP;
	return (RTLIB_ExitCode_t)chResp.result;

}

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_ScheduleRequest(pregExCtx_t prec) {
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

	DB(fprintf(stderr, FD("Schedule request for EXC [%d:%d]...\n"),
				rf_EXC_SCHEDULE.pyl.hdr.app_pid,
				rf_EXC_SCHEDULE.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(EXC_SCHEDULE);

	DB(fprintf(stderr, FD("Waiting BBQUE response...\n")));

	WAIT_RPC_RESP;
	return (RTLIB_ExitCode_t)chResp.result;
}

void BbqueRPC_FIFO_Client::_Exit() {
	ChannelRelease();
}


/******************************************************************************
 * Synchronization Protocol Messages - PreChange
 ******************************************************************************/

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_SyncpPreChangeResp(
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

	DB(fprintf(stderr, FD("PreChange response EXC [%d:%d] "
					"latency [%d]...\n"),
				rf_BBQ_SYNCP_PRECHANGE_RESP.pyl.hdr.app_pid,
				rf_BBQ_SYNCP_PRECHANGE_RESP.pyl.hdr.exc_id,
				rf_BBQ_SYNCP_PRECHANGE_RESP.pyl.syncLatency));

	// Sending RPC Request
	RPC_FIFO_SEND(BBQ_SYNCP_PRECHANGE_RESP);

	return RTLIB_OK;
}

void BbqueRPC_FIFO_Client::RpcBbqSyncpPreChange() {
	rpc_msg_BBQ_SYNCP_PRECHANGE_t msg;
	size_t bytes;

	// Read response RPC header
	bytes = ::read(client_fifo_fd, (void*)&msg,
			RPC_PKT_SIZE(BBQ_SYNCP_PRECHANGE));
	if (bytes <= 0) {
		fprintf(stderr, FE("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		chResp.result = RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}

	// Notify the Pre-Change
	SyncP_PreChangeNotify(msg);

}


/******************************************************************************
 * Synchronization Protocol Messages - SyncChange
 ******************************************************************************/

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_SyncpSyncChangeResp(
		rpc_msg_token_t token, pregExCtx_t prec, RTLIB_ExitCode_t sync) {

	rpc_fifo_BBQ_SYNCP_SYNCCHANGE_RESP_t rf_BBQ_SYNCP_SYNCCHANGE_RESP = {
		{
			FIFO_PKT_SIZE(BBQ_SYNCP_SYNCCHANGE_RESP),
			FIFO_PYL_OFFSET(BBQ_SYNCP_SYNCCHANGE_RESP),
			RPC_BBQ_RESP
		},
		{
			{
				RPC_BBQ_RESP,
				token,
				chTrdPid,
				prec->exc_id
			},
			(uint8_t)sync
		}
	};

	// Check that the ExitCode can be represented by the response message
	assert(sync < 256);

	DB(fprintf(stderr, FD("SyncChange response EXC [%d:%d]...\n"),
				rf_BBQ_SYNCP_SYNCCHANGE_RESP.pyl.hdr.app_pid,
				rf_BBQ_SYNCP_SYNCCHANGE_RESP.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(BBQ_SYNCP_SYNCCHANGE_RESP);

	return RTLIB_OK;
}

void BbqueRPC_FIFO_Client::RpcBbqSyncpSyncChange() {
	rpc_msg_BBQ_SYNCP_SYNCCHANGE_t msg;
	size_t bytes;

	// Read response RPC header
	bytes = ::read(client_fifo_fd, (void*)&msg,
			RPC_PKT_SIZE(BBQ_SYNCP_SYNCCHANGE));
	if (bytes <= 0) {
		fprintf(stderr, FE("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		chResp.result = RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}

	// Notify the Sync-Change
	SyncP_SyncChangeNotify(msg);

}


/******************************************************************************
 * Synchronization Protocol Messages - SyncChange
 ******************************************************************************/

void BbqueRPC_FIFO_Client::RpcBbqSyncpDoChange() {
	rpc_msg_BBQ_SYNCP_DOCHANGE_t msg;
	size_t bytes;

	// Read response RPC header
	bytes = ::read(client_fifo_fd, (void*)&msg,
			RPC_PKT_SIZE(BBQ_SYNCP_DOCHANGE));
	if (bytes <= 0) {
		fprintf(stderr, FE("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		chResp.result = RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}

	// Notify the Sync-Change
	SyncP_DoChangeNotify(msg);

}


/******************************************************************************
 * Synchronization Protocol Messages - PostChange
 ******************************************************************************/

RTLIB_ExitCode_t BbqueRPC_FIFO_Client::_SyncpPostChangeResp(
		rpc_msg_token_t token, pregExCtx_t prec, RTLIB_ExitCode_t result) {

	rpc_fifo_BBQ_SYNCP_POSTCHANGE_RESP_t rf_BBQ_SYNCP_POSTCHANGE_RESP = {
		{
			FIFO_PKT_SIZE(BBQ_SYNCP_POSTCHANGE_RESP),
			FIFO_PYL_OFFSET(BBQ_SYNCP_POSTCHANGE_RESP),
			RPC_BBQ_RESP
		},
		{
			{
				RPC_BBQ_RESP,
				token,
				chTrdPid,
				prec->exc_id
			},
			(uint8_t)result
		}
	};

	// Check that the ExitCode can be represented by the response message
	assert(result < 256);

	DB(fprintf(stderr, FD("PostChange response EXC [%d:%d]...\n"),
				rf_BBQ_SYNCP_POSTCHANGE_RESP.pyl.hdr.app_pid,
				rf_BBQ_SYNCP_POSTCHANGE_RESP.pyl.hdr.exc_id));

	// Sending RPC Request
	RPC_FIFO_SEND(BBQ_SYNCP_POSTCHANGE_RESP);

	return RTLIB_OK;
}

void BbqueRPC_FIFO_Client::RpcBbqSyncpPostChange() {
	rpc_msg_BBQ_SYNCP_POSTCHANGE_t msg;
	size_t bytes;

	// Read response RPC header
	bytes = ::read(client_fifo_fd, (void*)&msg,
			RPC_PKT_SIZE(BBQ_SYNCP_POSTCHANGE));
	if (bytes <= 0) {
		fprintf(stderr, FE("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		chResp.result = RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}

	// Notify the Sync-Change
	SyncP_PostChangeNotify(msg);

}



} // namespace rtlib

} // namespace bbque

