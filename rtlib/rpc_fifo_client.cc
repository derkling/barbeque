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
 * =====================================================================================
 */

#include "bbque/rtlib/rpc_fifo_client.h"

namespace bbque { namespace rtlib {

BbqueRPC_FIFO_Client::BbqueRPC_FIFO_Client() :
	BbqueRPC() {

}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Init(
			const char *name) {
	//Silence "args not used" warning.
	(void)name;

	return RTLIB_OK;
}

RTLIB_ExecutionContextHandler BbqueRPC_FIFO_Client::_Register(
			const char* name,
			const RTLIB_ExecutionContextParams* params) {
	//Silence "args not used" warning.
	(void)name;
	(void)params;

	return NULL;
}

void BbqueRPC_FIFO_Client::_Unregister(
			const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Start(
			const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Stop(
			const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Set(
			const RTLIB_ExecutionContextHandler ech,
			RTLIB_Constraint* constraints,
			uint8_t count) {
	//Silence "args not used" warning.
	(void)ech;
	(void)constraints;
	(void)count;

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Clear(
			const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;

	return RTLIB_OK;
}

} // namespace rtlib

} // namespace bbque

