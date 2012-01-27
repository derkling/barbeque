/**
 *       @file  rpc_proxy.h
 *      @brief  The proxy interface to the Barbeque communication channel IF
 *
 * This defines the a proxy class use to provide message queuing support by
 * wrapping the RPCChannelIF. This class unloads the channel modules
 * from the message queuing management, thus allowing for a simpler
 * implementaton. Meanwhile, this is class provides all the code to manage
 * message queuing and dequenung policies.
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

#ifndef BBQUE_RPC_PROXY_H_
#define BBQUE_RPC_PROXY_H_

#include "bbque/plugins/rpc_channel.h"
#include "bbque/plugins/logger.h"

#include "bbque/utils/metrics_collector.h"

#include <memory>
#include <mutex>
#include <thread>
#include <queue>

using bbque::plugins::RPCChannelIF;
using bbque::utils::MetricsCollector;

namespace bbque {

/**
 * @brief Queuing support to the low-level communication interface.
 */
class RPCProxy : public RPCChannelIF {

public:

	/**
	 * 
	 */
	static RPCProxy *GetInstance(std::string const & id);

	/**
	 * @brief Destructor
	 */
	~RPCProxy();

	/**
	 * @brief Initialize the communication channel.
	 */
	virtual int Init();

	/**
	 * @brief Get a pointer to the next message buffer.
	 *
	 * This methods blocks the caller until a new message is available and
	 * then return a pointer to the beginning of the message buffer and the
	 * size of the returned buffer.
	 *
	 * @note If more than one message are waiting to be processed, the message
	 * returned depends on the dequeuing rules defined by the implementation
	 * of this class.
	 */
	virtual size_t RecvMessage(rpc_msg_ptr_t & msg);

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
	 *
	 * @note this call is forwarded directly to the low-level channel module.
	 */
	virtual plugin_data_t GetPluginData(rpc_msg_ptr_t & msg);

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
	 *
	 * @note this call is forwarded directly to the low-level channel module.
	 */
	virtual void ReleasePluginData(plugin_data_t & pd);

	/**
	 * @brief Send a message buffer to the specified application.
	 *
	 * This method blocks the caller until the specified message buffer could
	 * be accepted for delivery to the specified application.
	 *
	 * @note this call is forwarded directly to the low-level channel module.
	 */
	virtual size_t SendMessage(plugin_data_t & pd, rpc_msg_ptr_t msg,
								size_t count);

	/**
	 * @brief Release the specified RPC message.
	 *
	 * This methods blocks the caller until a new message is available and
	 * then return a pointer to the beginning of the message buffer and the
	 * size of the returned buffer.
	 */
	virtual void FreeMessage(rpc_msg_ptr_t & msg);


private:

	/**
	 * 
	 */
	static RPCProxy *instance;

	MetricsCollector & mc;

	static bool channelLoaded;

	/**
	 * 
	 */
	typedef std::pair<rpc_msg_ptr_t, size_t> channel_msg_t;

	/**
	 * 
	 */
	class RPCMsgCompare {
	public:
		bool operator() (const channel_msg_t & lhs,
				const channel_msg_t & rhs) const;
	};

	/**
	 * 
	 */
	pid_t emTrdPid;

	/**
	 * 
	 */
	std::mutex trdStatus_mtx;

	/**
	 * 
	 */
	std::condition_variable trdStarted_cv;


	/**
	 * 
	 */
	bool done;

	/**
	 * @brief System logger instance
	 */
	plugins::LoggerIF *logger;

	/**
	 * 
	 */
	std::unique_ptr<RPCChannelIF> rpc_channel;

	/**
	 * 
	 */
	std::thread msg_fetch_trd;

	/**
	 * 
	 */
	std::priority_queue<channel_msg_t,
		std::vector<channel_msg_t>,
		RPCMsgCompare> msg_queue;

	/**
	 * 
	 */
	std::mutex msg_queue_mtx;

	/**
	 * 
	 */
	std::condition_variable queue_ready_cv;

	typedef enum RpcPrxMetrics {
		//----- Event counting metrics
		RP_BYTES_TX = 0,
		RP_BYTES_RX,
		RP_MSGS_TX,
		RP_MSGS_RX,
		//----- Couting statistics
		RP_RX_QUEUE,

		RP_METRICS_COUNT
	} RpcPrxMetrics_t;

	static MetricsCollector::MetricsCollection_t metrics[RP_METRICS_COUNT];
	/**
	 * 
	 */
	RPCProxy(std::string const & id);

	/**
	 * @brief Enqueue a new received message.
	 *
	 * Provides an enqueuing thread which continuously fetch messages from the
	 * low-level channel module and enqueue them to the proper queue.
	 */
	void EnqueueMessages();

};

} // namespace bbque

#endif // BBQUE_RPC_PROXY_H_

