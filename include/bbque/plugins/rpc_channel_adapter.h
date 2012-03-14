/*
 * Copyright (C) 2012  Politecnico di Milano
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BBQUE_RPC_CHANNEL_ADAPTER_H_
#define BBQUE_RPC_CHANNEL_ADAPTER_H_

#include "bbque/plugins/rpc_channel.h"
#include "bbque/plugins/rpc_channel_c.h"
#include "bbque/plugins/plugin.h"

namespace bbque { namespace plugins {

/**
 * @brief A C adapter for an RPC channel
 *
 * This class provides a wrapper to adapt C-coded RPCChannelIF modules.
 */
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

	ssize_t RecvMessage(rpc_msg_ptr_t & msg) {
		return rc->RecvMessage((void*)msg);
	}

	plugin_data_t GetPluginData(rpc_msg_ptr_t & msg) {
		return plugin_data_t(rc->GetPluginData((void*)msg));
	}

	void ReleasePluginData(plugin_data_t & pd) {
		rc->ReleasePluginData(pd.get());
	}

	ssize_t SendMessage(plugin_data_t & pd, rpc_msg_ptr_t msg,
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
