/**
 *       @file  rpc_proxy.cc
 *      @brief  The proxy interface to the Barbeque communication channel IF
 *
 * This defines the a proxy class use to provide message queuing support by
 * wrapping the RPCChannelIF. This class unloads the channel modules
 * from the message queuing management, thus allowing for a simpler
 * implementaton. Meanwhile, this is class provides all the code to manage
 * message queuing and dequenung policies.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
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

#include "bbque/rpc_proxy.h"

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/plugins/object_adapter.h"

#include "bbque/plugins/rpc_channel_adapter.h"
#include "bbque/utils/utility.h"

#include <signal.h>

namespace bp = bbque::plugins;

namespace bbque {

/**
 * Specialize the ObjectAdapter template for RPCChannel plugins
 */
typedef bp::ObjectAdapter<bp::RPCChannelAdapter, C_RPCChannel>
	RPCChannel_ObjectAdapter;

RPCProxy::RPCProxy(std::string const &id) :
	done(false) {

	// Get a logger
	plugins::LoggerIF::Configuration conf(RPC_CHANNEL_NAMESPACE".prx");
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	assert(logger!=NULL);

	// Build a object adapter for the Logger
	logger->Debug("PRXY RPC: RPC channel loading...");
	RPCChannel_ObjectAdapter rcoa;
	void* module = bp::PluginManager::GetInstance().
						CreateObject(id, NULL, &rcoa);
	if (!module) {
		logger->Fatal("PRXY RPC: RPC channel load FAILED");
		rpc_channel = std::unique_ptr<RPCChannelIF>();
		return;
	}

	// Keep track of the low-level channel module
	channelLoaded = true;
	rpc_channel = std::unique_ptr<RPCChannelIF>((RPCChannelIF*)module);

}

RPCProxy::~RPCProxy() {
	std::unique_lock<std::mutex> trdStatus_ul(trdStatus_mtx);

	if (rpc_channel) {
		//FIXME add plugins release code
		//bp::PluginManager::GetInstance().
		//	DeleteObject((void*)(rpc_channel.get()));
	}

	done = true;
	::kill(emTrdPid, EINTR);

}

bool RPCProxy::channelLoaded = false;
RPCProxy *RPCProxy::instance = NULL;
RPCProxy *RPCProxy::GetInstance(std::string const & id) {

	if (instance)
		return instance;

	instance = new RPCProxy(id);

	if (!channelLoaded) {
		delete instance;
		return NULL;
	}

	return instance;
}

int RPCProxy::Init() {
	std::unique_lock<std::mutex> trdStatus_ul(trdStatus_mtx);

	assert(rpc_channel);
	if (rpc_channel->Init())
		return -1;

	// Intialize message queues
	logger->Debug("Using (dummy) message priority based on RPC message ID");

	// Spawn the Enqueuing thread
	msg_fetch_trd = std::thread(&RPCProxy::EnqueueMessages, this);
	trdStarted_cv.wait(trdStatus_ul);

	return 0;
}

size_t RPCProxy::RecvMessage(rpc_msg_ptr_t & msg) {
	std::unique_lock<std::mutex> queue_status_ul(msg_queue_mtx);
	channel_msg_t ch_msg;
	size_t size;

	if (!msg_queue.size()) {
		// Wait for at least one element being pushed into the queue
		logger->Debug("PRXY RPC: waiting for new message");
		queue_ready_cv.wait(queue_status_ul);
	}

	// Dequeue received message
	ch_msg = msg_queue.top();
	msg_queue.pop();
	queue_status_ul.unlock();

	// Setup return values
	msg = ch_msg.first;
	size = ch_msg.second;

	logger->Debug("PRXY RPC: dq message [sze: %d]", size);

	return size;
}

bp::RPCChannelIF::plugin_data_t RPCProxy::GetPluginData(
		rpc_msg_ptr_t & msg) {
	return rpc_channel->GetPluginData(msg);
}

void RPCProxy::ReleasePluginData(plugin_data_t & pd) {
	return rpc_channel->ReleasePluginData(pd);
}

size_t RPCProxy::SendMessage(plugin_data_t & pd,
		rpc_msg_ptr_t msg, size_t count) {
	return rpc_channel->SendMessage(pd, msg, count);
}

bool RPCProxy::RPCMsgCompare::operator() (
		const channel_msg_t & lhs, const channel_msg_t & rhs) const {
	const rpc_msg_ptr_t hdr1 = lhs.first;
	const rpc_msg_ptr_t hdr2 = rhs.first;

	// Dummy policy: give priority to responses (RPC_BBQ_* messages)
	if (hdr1->typ > hdr2->typ)
		return true;

	return false;
}

void RPCProxy::EnqueueMessages() {
	size_t size;
	rpc_msg_ptr_t msg;
	std::unique_lock<std::mutex> queue_status_ul(
			msg_queue_mtx, std::defer_lock);

	// get the thread ID for further management
	emTrdPid = gettid();
	trdStarted_cv.notify_one();

	logger->Info("PRXY RPC: message fetcher started");
	while (!done) {
		// Wait for a new message being ready
		size = rpc_channel->RecvMessage(msg);
		if (size==EINTR)
			break;
		assert(msg);

		logger->Debug("PRXY RPC: RX [typ: %d, sze: %d]",
				msg->typ, size);

		// Enqueue the message
		queue_status_ul.lock();
		msg_queue.push(channel_msg_t(msg, size));
		queue_status_ul.unlock();
		queue_ready_cv.notify_one();

		logger->Debug("PRXY RPC: eq message [count: %d]",
				msg_queue.size());
	}
}

void RPCProxy::FreeMessage(rpc_msg_ptr_t & msg) {
	rpc_channel->FreeMessage(msg);
}

} // namespace bbque

