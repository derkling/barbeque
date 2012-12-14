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

#ifndef BBQUE_RPC_FIFO_CLIENT_H_
#define BBQUE_RPC_FIFO_CLIENT_H_

#include "bbque/rtlib.h"

#include "bbque/rtlib/bbque_rpc.h"
#include "bbque/rtlib/rpc_messages.h"
#include "bbque/rtlib/rpc_fifo_server.h"
#include "bbque/cpp11/condition_variable.h"
#include "bbque/cpp11/thread.h"

#include <sys/epoll.h>


namespace bbque { namespace rtlib {

/**
 * @brief Client side of the RPF FIFO channel
 *
 * Definition of the RPC protocol based on UNIX FIFOs to implement the
 * Barbeque communication channel. This defines the communication protocol in
 * terms of message format and functionalities.
 * The communication protocol must be aligend with the RTLib supported
 * services.
 *
 * @see bbque/rtlib.h
 * @see bbque/rtlib/rpc_messages.h
 */
class BbqueRPC_FIFO_Client : public BbqueRPC {

public:

	BbqueRPC_FIFO_Client();

	~BbqueRPC_FIFO_Client();

protected:

	RTLIB_ExitCode_t _Init(const char *name);

	RTLIB_ExitCode_t _Register(pregExCtx_t prec);

	RTLIB_ExitCode_t _Unregister(pregExCtx_t prec);

	RTLIB_ExitCode_t _Enable(pregExCtx_t prec);

	RTLIB_ExitCode_t _Disable(pregExCtx_t prec);

	RTLIB_ExitCode_t _ScheduleRequest(pregExCtx_t prec);

	RTLIB_ExitCode_t _Set(pregExCtx_t prec,
			RTLIB_Constraint* constraints, uint8_t count);

	RTLIB_ExitCode_t _Clear(pregExCtx_t prec);

	RTLIB_ExitCode_t _GGap(pregExCtx_t prec, uint8_t gap);

	void _Exit();

	inline uint32_t RpcMsgToken() {
		return chTrdPid;
	}

/******************************************************************************
 * Synchronization Protocol Messages
 ******************************************************************************/
	
	RTLIB_ExitCode_t _SyncpPreChangeResp(
			rpc_msg_token_t token,
			pregExCtx_t prec,
			uint32_t syncLatency);

	RTLIB_ExitCode_t _SyncpSyncChangeResp(
			rpc_msg_token_t token,
			pregExCtx_t prec,
			RTLIB_ExitCode_t sync);

	RTLIB_ExitCode_t _SyncpPostChangeResp(
			rpc_msg_token_t token,
			pregExCtx_t prec,
			RTLIB_ExitCode_t result);

private:

	char app_fifo_filename[BBQUE_FIFO_NAME_LENGTH];

	std::string app_fifo_path;

	std::string bbque_fifo_path;

	int client_fifo_fd;

	int server_fifo_fd;

	bool done;

	bool running;

	std::thread ChTrd;

	std::mutex trdStatus_mtx;

	std::condition_variable trdStatus_cv;

	/**
	 * @brief Serialize sending of command using the library
	 *
	 * The current implementation of the library allows to send a single
	 * command at each time for single library instance. This is required do
	 * properly handle responses from Barbque.
	 * This mutex should be used to protect the chResp responce attribute,
	 * which is always set to the last received response from Barbques.
	 *
	 * @see chResp
	 */
	std::mutex chCommand_mtx;

	/**
	 * @brief Signal the reception of a response from Barbeque
	 *
	 * Each time a new message has been received from Barbeque by the channel
	 * fetch thread, this variable is notified. Thus, commands could wait for
	 * a response by susepnding on it.
	 */
	std::condition_variable chResp_cv;

	/**
	 * @brief The last response reveiced by Barbeque
	 *
	 * This attribute should be always protected by the chCommand_mtx
	 */
	rpc_msg_resp_t chResp;

	RTLIB_ExitCode_t ChannelRelease();

	RTLIB_ExitCode_t ChannelSetup();

	RTLIB_ExitCode_t ChannelPair(const char *name);

	void ChannelFetch();

	void ChannelTrd(const char *name);

	void RpcBbqResp();

	/**
	 * @brief Get from FIFO a PreChange RPC message
	 */
	void RpcBbqSyncpPreChange();

	/**
	 * @brief Get from FIFO a SyncChange RPC message
	 */
	void RpcBbqSyncpSyncChange();

	/**
	 * @brief Get from FIFO a DoChange RPC message
	 */
	void RpcBbqSyncpDoChange();

	/**
	 * @brief Get from FIFO a PostChange RPC message
	 */
	void RpcBbqSyncpPostChange();

};

} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_FIFO_CLIENT_H_
