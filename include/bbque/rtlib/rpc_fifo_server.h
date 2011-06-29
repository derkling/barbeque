/**
 *       @file  rpc_fifo_server.h
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

#ifndef BBQUE_RPC_FIFO_SERVER_H_
#define BBQUE_RPC_FIFO_SERVER_H_

#include "bbque/rtlib.h"

#include "bbque/rtlib/rpc_messages.h"
#include "bbque/utils/utility.h"

#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#ifdef BBQUE_DEBUG
# define BBQUE_PUBLIC_FIFO_PATH "/tmp/bbque"
#else
# define BBQUE_PUBLIC_FIFO_PATH "/var/bbque"
#endif
#define BBQUE_PUBLIC_FIFO "rpc_fifo"

#define BBQUE_FIFO_NAME_LENGTH 32

#define BBQUE_RPC_FIFO_MAJOR_VERSION 1
#define BBQUE_RPC_FIFO_MINOR_VERSION 0

#define FIFO_PKT_SIZE(RPC_TYPE)\
	sizeof(bbque::rtlib::rpc_fifo_ ## RPC_TYPE ## _t)
#define FIFO_PYL_OFFSET(RPC_TYPE)\
	offsetof(bbque::rtlib::rpc_fifo_ ## RPC_TYPE ## _t, pyl)

namespace bbque { namespace rtlib {

/**
 * @brief The RPC FIFO message header
 */
typedef struct rpc_fifo_header {
	/** The bytes of the FIFO message */
	uint16_t fifo_msg_size;
	/** The offset of the RPC message start */
	uint8_t rpc_msg_offset;
	/** The type of the RPC message */
	uint8_t rpc_msg_type;
} rpc_fifo_header_t;

typedef struct rpc_fifo_GENERIC {
	/** The RPC fifo command header */
	rpc_fifo_header_t hdr;
	/** The RPC message payload */
	rpc_msg_header_t pyl;
} rpc_fifo_GENERIC_t;

#define RPC_FIFO_DEFINE_MESSAGE(RPC_TYPE)\
typedef struct rpc_fifo_ ## RPC_TYPE {\
	rpc_fifo_header_t hdr;\
	rpc_msg_ ## RPC_TYPE ## _t pyl;\
} rpc_fifo_ ## RPC_TYPE ## _t


/******************************************************************************
 * Channel Management
 ******************************************************************************/

/**
 * @brief An RPC_APP_PAIR FIFO command.
 *
 * This command is used by the FIFO communication channel to send the
 * application endpoit required to setup the communication channel.
 * The applicaiton end-point is defined by a fifo file node to be used to send
 * commands to the application.
 *
 * @note The only RPC command which contains communication channel specific
 * inforamtion is the RPC_EXC_PARI. All other commands maps on the
 * rpc_fifo_undef_t type.
 */
typedef struct rpc_fifo_app_pair {
	/** The RPC fifo command header */
	rpc_fifo_header_t header;
	/** The name of the application private fifo */
	char rpc_fifo[BBQUE_FIFO_NAME_LENGTH];
} rpc_fifo_app_pair_t;

/**
 * @brief An undefined FIFO message.
 *
 * All the RPC commands that does not carry channel specific information, are
 * mapped with this data structure.
 */
typedef struct rpc_fifo_undef {
	/** The RPC fifo command header */
	rpc_fifo_header_t header;
} rpc_fifo_undef_t;

/******************************************************************************
 * Execution Context Requests
 ******************************************************************************/

RPC_FIFO_DEFINE_MESSAGE(EXC_REGISTER);
RPC_FIFO_DEFINE_MESSAGE(EXC_UNREGISTER);
RPC_FIFO_DEFINE_MESSAGE(EXC_START);
RPC_FIFO_DEFINE_MESSAGE(EXC_STOP);
RPC_FIFO_DEFINE_MESSAGE(EXC_SCHEDULE);

RPC_FIFO_DEFINE_MESSAGE(BBQ_SYNCP_PRECHANGE);
RPC_FIFO_DEFINE_MESSAGE(BBQ_SYNCP_PRECHANGE_RESP);


/******************************************************************************
 * Barbeque Commands
 ******************************************************************************/

RPC_FIFO_DEFINE_MESSAGE(BBQ_STOP);



} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_FIFO_SERVER_H_

