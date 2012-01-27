/**
 *       @file  rpc_channel_c.h
 *      @brief  The C object model for barbeque RPCChannel plugins
 *
 * This defines the interface for RPCChannel barbque C based plugin. This file
 * provided the C based object model for RPCChannel plugins.
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

#ifndef BBQUE_RPC_CHANNEL_C_H_
#define BBQUE_RPC_CHANNEL_C_H_

//----- TestModule C interface
typedef struct C_RPCChannelHandle_ { char c; } * C_RPCChannelHandle;
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

