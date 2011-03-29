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
 * =====================================================================================
 */

#ifndef BBQUE_RPC_FIFO_SERVER_H_
#define BBQUE_RPC_FIFO_SERVER_H_

#include "bbque/rtlib.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <sys/syscall.h>

#define BBQUE_PUBLIC_FIFO_PATH "/var/bbque"
#define BBQUE_PUBLIC_FIFO "rpc_fifo"

#define BBQUE_FIFO_NAME_LENGTH 16

#define gettid() syscall(SYS_gettid)

namespace bbque { namespace rtlib {

class BbqueRPC_FIFO_Server {

};

} // namespace rtlib

} // namespace bbque

#endif // BBQUE_RPC_FIFO_SERVER_H_

