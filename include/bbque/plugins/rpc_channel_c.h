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

#ifndef BBQUE_RPC_CHANNEL_C_H_
#define BBQUE_RPC_CHANNEL_C_H_

//----- TestModule C interface
typedef struct C_RPCChannelHandle_ { char c; } * C_RPCChannelHandle;

/**
 * @brief A C coded RPC Channel
 *
 * A C based object model for RPCChannelIF plugins.
 */
typedef struct C_RPCChannel_ {
	int    (*Init)(void);
	size_t (*RecvMessage)(void *buff_ptr);
	void*  (*GetPluginData)(void *buff_ptr);
	void   (*ReleasePluginData)(void *pd);
	size_t (*SendMessage)(void *pd, void *buff_ptr, size_t count);
	void   (*FreeMessage)(void *buff_ptr);
	C_RPCChannelHandle handle;
} C_RPCChannel;

#endif // BBQUE_RPC_CHANNEL_C_H_
