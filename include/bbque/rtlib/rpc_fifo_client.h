/**
 *       @file  rpc_fifo.h
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

#ifndef BBQUE_RPC_FIFO_CLIENT_H_
#define BBQUE_RPC_FIFO_CLIENT_H_

#include "bbque/rtlib.h"

#include "bbque/rtlib/bbque_rpc.h"
#include "bbque/rtlib/rpc_messages.h"
#include "bbque/rtlib/rpc_fifo_server.h"
#include "bbque/utils/utility.h"

#include <sys/epoll.h>
#include <thread>

namespace bbque { namespace rtlib {

class BbqueRPC_FIFO_Client : public BbqueRPC {

public:

	BbqueRPC_FIFO_Client();

	~BbqueRPC_FIFO_Client();

protected:

	RTLIB_ExitCode _Init(
			const char *name);

	RTLIB_ExitCode _Register(pregExCtx_t pregExCtx);

	RTLIB_ExitCode _Unregister(pregExCtx_t pregExCtx);

	RTLIB_ExitCode _Start(pregExCtx_t pregExCtx);

	RTLIB_ExitCode _Stop(pregExCtx_t pregExCtx);

	RTLIB_ExitCode _GetWorkingMode(
			pregExCtx_t prec,
			RTLIB_WorkingModeParams *wm);

	RTLIB_ExitCode _Set(
			const RTLIB_ExecutionContextHandler ech,
			RTLIB_Constraint* constraints,
			uint8_t count);

	RTLIB_ExitCode _Clear(
			const RTLIB_ExecutionContextHandler ech);


	void _Exit();

private:

	char app_fifo_filename[BBQUE_FIFO_NAME_LENGTH];

	std::string app_fifo_path;

	std::string bbque_fifo_path;

	int client_fifo_fd;

	int server_fifo_fd;

	int epoll_fd;

	struct epoll_event epoll_ev;

	#define MAX_EPOLL_EVENTS 1
	struct epoll_event epoll_evts[MAX_EPOLL_EVENTS];

	pid_t chTrdPid;

	std::thread ChTrd;

	std::mutex trdStatus_mtx;

	std::condition_variable trdStarted_cv;

	std::mutex chSetup_mtx;

	std::condition_variable chSetup_cv;

	RTLIB_ExitCode ChannelRelease();

	RTLIB_ExitCode ChannelPair(const char *name);

	RTLIB_ExitCode ChannelSetup(const char *name);

	void ChannelTrd();

	RTLIB_ExitCode WaitBbqueResp(int ms = 500);

	RTLIB_ExitCode BbqueResult();
};

} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_FIFO_CLIENT_H_

