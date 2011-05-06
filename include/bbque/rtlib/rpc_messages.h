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

#define RPC_PKT_SIZE(type) sizeof(bbque::rtlib::rpc_msg_##type##_t)

namespace bbque { namespace rtlib {

/**
 * @brief The RPC message identifier
 *
 * The value of the message identifier is used to give priority to messages.
 * The higer the message id the higer the message priority.
 */
typedef enum rpc_msg_type {
	//--- Execution Context Originated Messages
	RPC_EXC_REGISTER,
	RPC_EXC_UNREGISTER,
	RPC_EXC_SET,
	RPC_EXC_CLEAR,
	RPC_EXC_START,
	RPC_EXC_STOP,
	RPC_EXC_RESP,
	RPC_EXC_MSGS_COUNT, ///< The number of EXC originated messages
	RPC_APP_PAIR,
	RPC_APP_EXIT,
	RPC_APP_RESP,
	RPC_APP_MSGS_COUNT, ///< The number of APP originated messages
	//--- Barbeque Originated Messages
	RPC_BBQ_RESP,
	RPC_BBQ_SET_WORKING_MODE,
	RPC_BBQ_STOP_EXECUTION,
	RPC_BBQ_MSGS_COUNT ///< The number of EXC originated messages
} rpc_msg_type_t;

/**
 * @brief The RPC message header
 */
typedef struct rpc_msg_header {
	/** The command to execute (defines the message "payload" type) */
	rpc_msg_type_t typ;

//FIXME These is maybe superflous... it is required just for the pairing
// Than it is better to exchange a communication token ;-)
	/** The application ID (thread ID) */
	pid_t app_pid;

// FIXME This is required just by EXC related messages: better to move there
	/** The execution context ID */
	uint8_t exc_id;

} rpc_msg_header_t;

/**
 * @brief The responce to a command
 */
typedef struct rpc_msg_resp {
	/** The RPC fifo command header */
	rpc_msg_header_t header;
	/** The RPC protocol major version */
	int8_t result;
} rpc_msg_resp_t;


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
} rpc_msg_exc_register_t;

/**
 * @brief Command to STOP an application execution context.
 */
typedef struct rpc_msg_bbq_stop {
	/** The RPC fifo command header */
	rpc_msg_header_t header;
	/** The Timeout for stopping the application */
	struct timespec timeout;
} rpc_msg_bbq_stop_t;

/**
 * @brief Command to notify an application is exiting.
 */
typedef rpc_msg_header_t rpc_msg_app_exit_t;


} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_MESSAGES_H_

