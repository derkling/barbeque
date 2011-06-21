/**
 *       @file  static_plugin.cc
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

#include "fifo_rpc.h"

#include "bbque/modules_factory.h"
#include "bbque/rtlib/rpc_fifo_server.h"

#include "bbque/config.h"
#include <boost/filesystem.hpp>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

namespace br = bbque::rtlib;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace bbque { namespace plugins {

FifoRPC::FifoRPC(std::string const & fifo_dir) :
	initialized(false),
	conf_fifo_dir(fifo_dir) {

	// Get a logger
	plugins::LoggerIF::Configuration conf(MODULE_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	assert(logger!=NULL);

}

FifoRPC::~FifoRPC() {

}

//----- RPCChannelIF module interface

int FifoRPC::Init() {
	int error;
	fs::path fifo_path(conf_fifo_dir);
	boost::system::error_code ec;

	if (initialized)
		return 0;

	logger->Debug("FIFO RPC: channel initialization...");

	fifo_path /= "/"BBQUE_PUBLIC_FIFO;
	logger->Debug("FIFO RPC: checking FIFO [%s]...",
			fifo_path.string().c_str());
	if (!fs::exists(fifo_path, ec)) {

		// Make dir if not already present
		logger->Debug("FIFO RPC: create dir [%s]...",
			fifo_path.parent_path().c_str());
		fs::create_directories(fifo_path.parent_path(), ec);

		// Create the server side pipe (if not already existing)
		logger->Debug("FIFO RPC: create FIFO [%s]...",
				fifo_path.string().c_str());
		error = ::mkfifo(fifo_path.string().c_str(), 0666);
		if (error) {
			logger->Error("FIFO RPC: RPC FIFO [%s] cration FAILED",
					fifo_path.string().c_str());
			return -1;
		}
	}

	// Ensuring we have a pipe
	if (fs::status(fifo_path, ec).type() != fs::fifo_file) {
		logger->Error("ERROR, RPC FIFO [%s] already in use",
				fifo_path.string().c_str());
		return -2;
	}

	// Opening the server side pipe (R/W to keep it opened)
	logger->Debug("FIFO RPC: opening (for read)...");
	rpc_fifo_fd = ::open(fifo_path.string().c_str(),
			O_RDWR);
			//O_RDONLY);
	if (rpc_fifo_fd < 0) {
		logger->Error("FAILED opening RPC FIFO [%s]",
					fifo_path.string().c_str());
		::unlink(fifo_path.string().c_str());
		return -3;
	}

	logger->Info("FIFO RPC: channel initialization DONE\n");
	return 0;
}

size_t FifoRPC::RecvMessage(rpc_msg_ptr_t & msg) {
	br::rpc_fifo_header_t hdr;
	int8_t *fifo_buff_ptr;
	size_t bytes;

	logger->Debug("FIFO RPC: waiting message...");

	// Wait for the next message being available
	// ... which always starts with the FIFO header
	bytes = ::read(rpc_fifo_fd, (void*)&hdr, sizeof(br::rpc_fifo_header_t));
	if (bytes<=0) {
		logger->Error("FIFO RPC: fifo read error");
		return bytes;
	}

	logger->Debug("FIFO RPC: RX[typ: %hhd, size: %hd, off: %hhd]",
			hdr.rpc_msg_type, hdr.fifo_msg_size, hdr.rpc_msg_offset);

	// Allocate a new message buffer
	fifo_buff_ptr = (int8_t*)::malloc(hdr.fifo_msg_size);
	if (!fifo_buff_ptr) {
		char c;
		logger->Error("FIFO RPC: message buffer creation FAILED");
		// Remove the remaining message from the FIFO
		for ( ; bytes<hdr.fifo_msg_size; bytes++) {
			::read(rpc_fifo_fd, (void*)&c, sizeof(char));
		}
		return 0;
	}

	// Save header into new buffer
	memcpy((void*)fifo_buff_ptr, &hdr, sizeof(br::rpc_fifo_header_t));

	// Copy the remaining RPC message on the FIFO message buffer
	::read(rpc_fifo_fd,
			((fifo_buff_ptr)+sizeof(br::rpc_fifo_header_t)),
			hdr.fifo_msg_size-sizeof(br::rpc_fifo_header_t));

	// Return a pointer to the new message
	msg = (rpc_msg_ptr_t)(fifo_buff_ptr+hdr.rpc_msg_offset);
	// TODO provide a HEXDUMP logging routine
	//logger->Debug("FIFO RPC: RX[buff: %s]", (char*)msg);

	return (hdr.fifo_msg_size-hdr.rpc_msg_offset);
}

RPCChannelIF::plugin_data_t FifoRPC::GetPluginData(
		rpc_msg_ptr_t & msg) {
	(void)msg;
	return std::shared_ptr<void>();
}

void FifoRPC::ReleasePluginData(plugin_data_t & pd) {
	(void)pd;
}

size_t FifoRPC::SendMessage(plugin_data_t & pd, rpc_msg_ptr_t & msg,
		size_t count) {
	(void)pd;
	(void)msg;
	(void)count;
	return 0;
}

void FifoRPC::FreeMessage(rpc_msg_ptr_t & msg) {
	size_t hdr_size;

	// Recover the beginning of the FIFO message
	switch (msg->msg_typ) {
	case br::RPC_EXC_PAIR:
		hdr_size = sizeof(br::rpc_fifo_exc_pair_t)-1;
		break;
	default:
		hdr_size = sizeof(br::rpc_fifo_undef_t)-1;
		break;
	}
	msg -= hdr_size;

	// Releaseing the FIFO message buffer
	::free(msg);
}

//----- static plugin interface

void * FifoRPC::Create(PF_ObjectParams *params) {
	static std::string conf_fifo_dir;

	// Declare the supported options
	po::options_description fifo_rpc_opts_desc("FIFO RPC Options");
	fifo_rpc_opts_desc.add_options()
		(MODULE_NAMESPACE".dir", po::value<std::string>
		 (&conf_fifo_dir)->default_value(BBQUE_PUBLIC_FIFO_PATH),
		 "path of the FIFO dir")
		;
	static po::variables_map fifo_rpc_opts_value;

	// Get configuration params
	PF_Service_ConfDataIn data_in;
	data_in.opts_desc = &fifo_rpc_opts_desc;
	PF_Service_ConfDataOut data_out;
	data_out.opts_value = &fifo_rpc_opts_value;
	PF_ServiceData sd;
	sd.id = MODULE_NAMESPACE;
	sd.request = &data_in;
	sd.response = &data_out;

	int32_t response = params->
		platform_services->InvokeService(PF_SERVICE_CONF_DATA, sd);
	if (response!=PF_SERVICE_DONE)
		return NULL;

	DB(fprintf(stderr, "FIFO RPC: using dir [%s]\n",
			conf_fifo_dir.c_str()));

	return new FifoRPC(conf_fifo_dir);

}

int32_t FifoRPC::Destroy(void *plugin) {
  if (!plugin)
    return -1;
  delete (FifoRPC *)plugin;
  return 0;
}

} // namesapce plugins

} // namespace bque

