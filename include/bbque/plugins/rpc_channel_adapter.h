/**
 *       @file  rpc_channel_adapter.h
 *      @brief  A C++ wrapper class for C based RPCChannelIF plugins
 *
 * This class provides a wrapper to adapt RPCChannel modules written in C.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  02/01/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_RPC_CHANNEL_ADAPTER_H_
#define BBQUE_RPC_CHANNEL_ADAPTER_H_

#include "bbque/plugins/rpc_channel.h"
#include "bbque/plugins/rpc_channel_c.h"
#include "bbque/plugins/plugin.h"

namespace bbque { namespace plugins {

class RPCChannelAdapter : public RPCChannelIF {

public:

	RPCChannelAdapter(C_RPCChannel * _rc, PF_DestroyFunc _df) :
		rc(_rc),
		df(_df) {
	}

	~RPCChannelAdapter() {
		if (df)
			df(rc);
	}

	int Init() {
		return rc->Init();
	}

	size_t RecvMessage(rpc_msg_ptr_t & msg) {
		return rc->RecvMessage((void*)msg);
	}

	plugin_data_t GetPluginData(rpc_msg_ptr_t & msg) {
		return plugin_data_t(rc->GetPluginData((void*)msg));
	}

	void ReleasePluginData(plugin_data_t & pd) {
		rc->ReleasePluginData(pd.get());
	}

	size_t SendMessage(plugin_data_t & pd, rpc_msg_ptr_t msg,
			size_t count) {
		return rc->SendMessage(pd.get(), msg, count);
	}

	void FreeMessage(rpc_msg_ptr_t & msg) {
		rc->FreeMessage((void*)msg);
	}

private:

	C_RPCChannel * rc;
	PF_DestroyFunc df;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_RPC_CHANNEL_ADAPTER_H_

