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
 * =============================================================================
 */

#ifndef BBQUE_RPC_MESSAGES_H_
#define BBQUE_RPC_MESSAGES_H_

#include "bbque/rtlib.h"

namespace bbque { namespace rtlib {

/**
 * @brief The RPC message identifier
 *
 * The value of the message identifier is used to give priority to messages.
 * The higer the message id the higer the message priority.
 */
typedef enum rpc_msg_type {
	//--- Execution Context Originated Messages
	RPC_EXC_PAIR = 0,
	RPC_EXC_REGISTER,
	RPC_EXC_UNREGISTER,
	RPC_EXC_SET,
	RPC_EXC_CLEAR,
	RPC_EXC_START,
	RPC_EXC_STOP,
	RPC_EXC_COMMANDS, ///< The number of EXC originated messages
	//--- Barbeque Originated Messages
	RPC_BBQ_SET_WORKING_MODE,
	RPC_BBQ_STOP_EXECUTION
} rpc_msg_type_t;

/**
 * @brief The RPC message header
 */
typedef struct rpc_msg_header {
	/** The command to execute (defines the message "payload" type) */
	rpc_msg_type_t msg_typ;
	/** The execution context ID (thread ID) */
	pid_t exc_id;
} rpc_msg_header_t;

/**
 * @brief Command to register a new execution context.
 */
typedef struct rpc_msg_app_pair {
	/** The RPC fifo command header */
	rpc_msg_header_t header;
	/** The RPC protocol major version */
	uint8_t mjr_version;
	/** The RPC protocol minor version */
	uint8_t mnr_version;
} rpc_msg_app_pair_t;

/**
 * @brief Command to register a new execution context.
 */
typedef struct rpc_msg_exc_register {
	/** The RPC fifo command header */
	rpc_msg_header_t header;
	/** The name of the registered execution context */
	char exc_name[RTLIB_EXC_NAME_LENGTH];
	/** The name of the required recipe */
	char recipe[RTLIB_RECIPE_NAME_LENGTH];
} rpc_fifo_exc_register_t;

} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_MESSAGES_H_

