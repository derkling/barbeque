/**
 *       @file  rpc_messages.h
 *      @brief  Data structures for message passing RPC frameworks
 *
 * Definition of data structures that can be used by a message based RPC
 * communicaiton protocol. Such messages are suitable for the implementation
 * of a Barbeque communication channel. Indeed, this files support the
 * definition of the communication protocol in terms of message format and
 * functionalities.  The communication protocol must be aligend with the RTLib
 * supported services.
 *
 * @see bbque/rtlib.h
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

#ifndef BBQUE_RPC_MESSAGES_H_
#define BBQUE_RPC_MESSAGES_H_

#include "bbque/rtlib.h"

namespace bbque { namespace rtlib {

/**
 * @brief The RPC message identifier
 */
typedef enum rpc_command {
	//--- Execution Context Originated Messages
	RPC_FIFO_EXC_REGISTER = 0,
	RPC_FIFO_EXC_SET_CONSTRAINT,
	RPC_FIFO_EXC_COMMANDS, ///< The number of EXC originated messages
	//--- Barbeque Originated Messages
	RPC_FIFO_BBQ_SET_WORKING_MODE,
	RPC_FIFO_BBQ_STOP_EXECUTION,
} rpc_fifo_command_t;

/**
 * @brief The RPC message header
 */
typedef struct rpc_msg_header {
	/** The execution context ID (thread ID) */
	pid_t exc_id;
	/** The command to execute (defines the message "payload" type) */
	rpc_command_t cmd_id;
} rpc_fifo_header_t;

/**
 * @brief Command to register a new execution context.
 */
typedef struct rpc_msg_exc_register {
	/** The RPC fifo command header */
	rpc_msg_header_t header;
	/** The name of the execution context private fifo */
	char rpc_fifo[BBQUE_FIFO_NAME_LENGTH];
	/** The name of the registered execution context */
	char exc_name[RTLIB_EXC_NAME_LENGTH];
	/** The name of the required recipe */
	char recipe[RTLIB_RECIPE_NAME_LENGTH];
} rpc_fifo_exc_register_t;

/**
 * @brief Command to set a working mode for an execution context.
 */
typedef struct rpc_msg_set_working_mode {
	/** The RPC fifo command header */
	rpc_msg_header_t header;
	/** The name of the registered execution context */
	char exc_name[RTLIB_EXC_NAME_LENGTH];
	/** The name of the required recipe */
	char wm_id[RTLIB_WM_ID_MAXLEN];
} rpc_fifo_bbq_set_working_mode_t;


} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_MESSAGES_H_

