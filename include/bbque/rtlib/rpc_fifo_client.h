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
 * =====================================================================================
 */

#ifndef BBQUE_RPC_FIFO_H_
#define BBQUE_RPC_FIFO_H_

#include "bbque/rtlib.h"

#include "bbque/rtlib/bbque_rpc.h"
#include "bbque/rtlib/rpc_fifo_client.h"
#include "bbque/rtlib/rpc_fifo_server.h"

namespace bbque { namespace rtlib {

class BbqueRPC_FIFO_Client : public BbqueRPC {

public:

	BbqueRPC_FIFO_Client();

	RTLIB_ExitCode _Init(
			const char *name);

	RTLIB_ExecutionContextHandler _Register(
			const char* name,
			const RTLIB_ExecutionContextParams* params);

	void _Unregister(
			const RTLIB_ExecutionContextHandler ech);

	RTLIB_ExitCode _Start(
			const RTLIB_ExecutionContextHandler ech);

	RTLIB_ExitCode _Stop(
			const RTLIB_ExecutionContextHandler ech);

	RTLIB_ExitCode _Set(
			const RTLIB_ExecutionContextHandler ech,
			RTLIB_Constraint* constraints,
			uint8_t count);

	RTLIB_ExitCode _Clear(
			const RTLIB_ExecutionContextHandler ech);


};

} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_FIFO_H_

