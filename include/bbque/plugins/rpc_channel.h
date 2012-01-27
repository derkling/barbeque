/**
 *       @file  rpc_channel.h
 *      @brief  The interface for a Barbeque communication Channel
 *
 * This defines the common interface for each communication channel module
 * which can be used by the Barbeque framework to interface with applications.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  01/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#ifndef BBQUE_RPC_CHANNEL_H_
#define BBQUE_RPC_CHANNEL_H_

#include "bbque/rtlib/rpc_messages.h"

#include <memory>

#define RPC_CHANNEL_NAMESPACE "bq.rpc"

using bbque::rtlib::rpc_msg_header_t;

namespace bbque { namespace plugins {

/**
 * @class ChannelIF
 * @brief A module to provide the low-level communication support to interface
 * with applications.
 */
class RPCChannelIF {

public:

	/**
	 * 
	 */
	typedef rpc_msg_header_t *rpc_msg_ptr_t;

	/**
	 * 
	 */
	typedef std::shared_ptr<void> plugin_data_t;


	/**
	 * @brief Initialize the communication channel.
	 */
	virtual int Init() = 0;

	/**
	 * @brief Get a pointer to the next message buffer.
	 *
	 * This methods blocks the caller until a new message is available and
	 * then return a pointer to the beginning of the message buffer and the
	 * size of the returned buffer.
	 */
	virtual size_t RecvMessage(rpc_msg_ptr_t & msg) = 0;

	/**
	 * @brief Get a pointer to plugins data.
	 *
	 * Based on the specified message buffer, the channel module could
	 * allocate and initialize a set of plugin specific data. These data are
	 * opaque to the Barbeque RTRM, but they will be passed to the plugin each
	 * time a message should be sent.
	 *
	 * This method is called only after the reception of a RPC_APP_PAIR
	 * message, which is passed to the communication channel module as a
	 * reference to map the new communication channel. A communication channel
	 * module could use these information (or its own specific information
	 * pre-pended to the specified message buffer) to setup a communication
	 * channel with the application and save all the required handler to the
	 * plugins data to be returned. These resource and correponding handler
	 * will be passed to the module each time a new message should be sent to
	 * an application.
	 *
	 * @see look at the BbqueRPC_FIFO_Server class for a usage example.
	 */
	virtual plugin_data_t GetPluginData(rpc_msg_ptr_t & msg) = 0;

	/**
	 * @brief Release plugins data.
	 *
	 * Ask the channel to release all the resources associated with the
	 * specified plugin data. This Barbeque RTRM opaque type is usually used
	 * to save the communication channel specific connection information. In
	 * this case, this method authorize the communicaiton channel module to
	 * close the corresponding connection and release all the resource.
	 *
	 * @see look at the BbqueRPC_FIFO_Server class for a usage example.
	 */
	virtual void ReleasePluginData(plugin_data_t & pd) = 0;

	/**
	 * @brief Send a message buffer to the specified application.
	 *
	 * This method blocks the caller until the specified message buffer could
	 * be accepted for delivery to the specified application.
	 */
	virtual size_t SendMessage(plugin_data_t & pd, rpc_msg_ptr_t msg,
								size_t count) = 0;

	/**
	 * @brief Release the specified RPC message.
	 *
	 * This methods blocks the caller until a new message is available and
	 * then return a pointer to the beginning of the message buffer and the
	 * size of the returned buffer.
	 */
	virtual void FreeMessage(rpc_msg_ptr_t & msg) = 0;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_RPC_CHANNEL_H_

