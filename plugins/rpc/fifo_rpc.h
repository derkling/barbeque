/**
 *       @file  static_plugin.h
 *      @brief  An example of static C++ plugin
 *
 * This defines a simple example of static C++ plugin which is intended both to
 * demostrate how to write them and to test the PluginManager implementation.
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

#ifndef BBQUE_PLUGINS_FIFO_RPC_H_
#define BBQUE_PLUGINS_FIFO_RPC_H_

#include "bbque/rtlib/rpc_fifo_server.h"

#include "bbque/plugins/rpc_channel.h"
#include "bbque/plugins/plugin.h"
#include "bbque/plugins/logger.h"

#include <boost/asio.hpp>
namespace ba = boost::asio;

#include <cstdint>

#define MODULE_NAMESPACE RPC_CHANNEL_NAMESPACE"fif"

// These are the parameters received by the PluginManager on create calls
struct PF_ObjectParams;

namespace bbque { namespace plugins {

/**
 * @brief A FIFO based implementation of the RPCChannelIF interface.
 *
 * This class provide a FIFO based communication channel between the Barbque
 * RTRM and the applications.
 */
class FifoRPC : public RPCChannelIF {

typedef struct fifo_data {
	/** The handler to the application FIFO */
	int app_fifo_fd;
	/** The application FIFO filename */
	char app_fifo_filename[BBQUE_FIFO_NAME_LENGTH];
} fifo_data_t;


public:

//----- static plugin interface

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	static void * Create(PF_ObjectParams *);

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	static int32_t Destroy(void *);

	~FifoRPC();

//----- RPCChannelIF module interface


	virtual size_t RecvMessage(rpc_msg_ptr_t & msg);

	virtual plugin_data_t GetPluginData(rpc_msg_ptr_t & msg);

	virtual void ReleasePluginData(plugin_data_t & pd);

	virtual size_t SendMessage(plugin_data_t & pd, rpc_msg_ptr_t msg,
								size_t count);

	virtual void FreeMessage(rpc_msg_ptr_t & msg);

private:



	/**
	 * @brief System logger instance
	 */
	plugins::LoggerIF *logger;

	/**
	 * @brief Thrue if the channel has been correctly initalized
	 */
	bool initialized;

	/**
	 * @brief The path of the directory for FIFOs creation
	 */
	std::string conf_fifo_dir;

	/**
	 * @brief The RPC server FIFO descriptor
	 */
	int rpc_fifo_fd;

	/**
	 * @brief   The plugins constructor
	 * Plugins objects could be build only by using the "create" method.
	 * Usually the PluginManager acts as object
	 * @param   
	 * @return  
	 */
	FifoRPC(std::string const & fifo_dir);

	int Init();

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_PLUGINS_TESTING_H_

