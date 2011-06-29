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

//--- Application Originated Messages
	RPC_APP_PAIR,
	RPC_APP_EXIT,

	RPC_APP_RESP, ///< Response to an APP request
	RPC_APP_MSGS_COUNT, ///< The number of APP originated messages

//--- Execution Context Originated Messages
	RPC_EXC_REGISTER,
	RPC_EXC_UNREGISTER,
	RPC_EXC_SET,
	RPC_EXC_CLEAR,
	RPC_EXC_START,
	RPC_EXC_STOP,
	RPC_EXC_SCHEDULE,

	RPC_EXC_RESP, ///< Response to an EXC request
	RPC_EXC_MSGS_COUNT, ///< The number of EXC originated messages

//--- Barbeque Originated Messages
	RPC_BBQ_STOP_EXECUTION,

	RPC_BBQ_SYNCP_PRECHANGE,
	RPC_BBQ_SYNCP_SYNCCHANGE,
	RPC_BBQ_SYNCP_DOCHANGE,
	RPC_BBQ_SYNCP_POSTCHANGE,

	RPC_BBQ_RESP, ///< Response to a BBQ command
	RPC_BBQ_MSGS_COUNT ///< The number of EXC originated messages

} rpc_msg_type_t;

typedef uint32_t rpc_msg_token_t;

/**
 * @brief The RPC message header
 */
typedef struct rpc_msg_header {
	/** The command to execute (defines the message "payload" type) */
	uint8_t typ;

	/** A token used by the message sender to match responses */
	rpc_msg_token_t token;

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
	rpc_msg_header_t hdr;
	/** The RTLIB command exit code */
	RTLIB_ExitCode result;
} rpc_msg_resp_t;


/******************************************************************************
 * Channel Management
 ******************************************************************************/

/**
 * @brief Command to register a new execution context.
 */
typedef struct rpc_msg_app_pair {
	/** The RPC fifo command header */
	rpc_msg_header_t hdr;
	/** The RPC protocol major version */
	uint8_t mjr_version;
	/** The RPC protocol minor version */
	uint8_t mnr_version;
	/** The name of the application */
	char app_name[RTLIB_APP_NAME_LENGTH];
} rpc_msg_app_pair_t;

/**
 * @brief Command to notify an application is exiting.
 */
typedef rpc_msg_header_t rpc_msg_app_exit_t;


/******************************************************************************
 * Execution Context Requests
 ******************************************************************************/

/**
 * @brief Command to register a new execution context.
 */
typedef struct rpc_msg_exc_register {
	/** The RPC fifo command header */
	rpc_msg_header_t hdr;
	/** The name of the registered execution context */
	char exc_name[RTLIB_EXC_NAME_LENGTH];
	/** The name of the required recipe */
	char recipe[RTLIB_RECIPE_NAME_LENGTH];
} rpc_msg_exc_register_t;

/**
 * @brief Command to unregister an execution context.
 */
typedef struct rpc_msg_exc_unregister {
	/** The RPC fifo command header */
	rpc_msg_header_t hdr;
	/** The name of the execution context */
	char exc_name[RTLIB_EXC_NAME_LENGTH];
} rpc_msg_exc_unregister_t;


/**
 * @brief Command to start an execution context.
 */
typedef struct rpc_msg_exc_start {
	/** The RPC fifo command header */
	rpc_msg_header_t hdr;
} rpc_msg_exc_start_t;

/**
 * @brief Command to stop an execution context.
 */
typedef struct rpc_msg_exc_stop {
	/** The RPC fifo command header */
	rpc_msg_header_t hdr;
} rpc_msg_exc_stop_t;

/**
 * @brief Command to ask for being scheduled.
 * 
 * This message is send by the RTLIB once an EXC ask the RTRM to be scheduled
 * (as soon as possible). The RTRM should identify the best AWM to be assigned
 * for the requesting execution context.
 */
typedef struct rpc_msg_exc_schedule {
	/** The RPC fifo command header */
	rpc_msg_header_t hdr;
} rpc_msg_exc_schedule_t;


/******************************************************************************
 * Synchronization Protocol Messages
 ******************************************************************************/

/**
 * @brief Synchronization Protocol PreChange command
 */
typedef struct rpc_msg_bbq_syncp_prechange {
	/** The RPC fifo command header */
	rpc_msg_header_t hdr;
	/** The selected AWM */
	uint16_t awm;
} rpc_msg_bbq_syncp_prechange_t;

/**
 * @brief Synchronization Protocol PreChange response
 */
typedef struct rpc_msg_bbq_syncp_prechange_resp {
	/** The RPC fifo command header */
	rpc_msg_header_t hdr;
	/** An extimation of the Synchronization Latency */
	uint32_t syncLatency;
} rpc_msg_bbq_syncp_prechange_resp_t;

/******************************************************************************
 * Barbeque Commands
 ******************************************************************************/

/**
 * @brief Command to STOP an application execution context.
 */
typedef struct rpc_msg_bbq_stop {
	/** The RPC fifo command header */
	rpc_msg_header_t hdr;
	/** The Timeout for stopping the application */
	struct timespec timeout;
} rpc_msg_bbq_stop_t;


} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_MESSAGES_H_

