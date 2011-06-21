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

#include <cstdlib>
#include <cstring>

#define BBQUE_PUBLIC_FIFO_PATH "/var/bbque"
#define BBQUE_PUBLIC_FIFO "rpc_fifo"

#define BBQUE_FIFO_NAME_LENGTH 32

#define BBQUE_RPC_FIFO_MAJOR_VERSION 1
#define BBQUE_RPC_FIFO_MINOR_VERSION 0

#define FIFO_PKT_SIZE(type) sizeof(bbque::rtlib::rpc_fifo_##type##_t)

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

/**
 * @brief An RPC_EXC_PAIR FIFO command.
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
typedef struct rpc_fifo_exc_pair {
	/** The RPC fifo command header */
	rpc_fifo_header_t header;
	/** The name of the application private fifo */
	char rpc_fifo[BBQUE_FIFO_NAME_LENGTH];
	/** The payload start */
	int8_t payload_start;
} rpc_fifo_exc_pair_t;

/**
 * @brief An undefined FIFO message.
 *
 * All the RPC commands that does not carry channel specific information, are
 * mapped with this data structure.
 */
typedef struct rpc_fifo_undef {
	/** The RPC fifo command header */
	rpc_fifo_header_t header;
	/** The payload start */
	int8_t payload_start;
} rpc_fifo_undef_t;


} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_FIFO_SERVER_H_

