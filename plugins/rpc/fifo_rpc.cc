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
	conf_fifo_dir(fifo_dir),
	rpc_fifo_fd(0) {

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
	logger->Debug("FIFO RPC: opening R/W...");
	rpc_fifo_fd = ::open(fifo_path.string().c_str(),
			O_RDWR);
	if (rpc_fifo_fd < 0) {
		logger->Error("FAILED opening RPC FIFO [%s]",
					fifo_path.string().c_str());
		rpc_fifo_fd = 0;
		::unlink(fifo_path.string().c_str());
		return -3;
	}

	// Marking channel al already initialized
	initialized = true;

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
	bytes = ::read(rpc_fifo_fd, (void*)&hdr, FIFO_PKT_SIZE(header));
	if (bytes<=0) {
		if (bytes==EINTR)
			logger->Debug("FIFO RPC: exiting FIFO read...");
		else
			logger->Error("FIFO RPC: fifo read error");
		return bytes;
	}

	logger->Debug("FIFO RPC: RX HDR [sze: %hd, off: %hhd, typ: %hhd]",
			hdr.fifo_msg_size, hdr.rpc_msg_offset, hdr.rpc_msg_type);

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
	::memcpy((void*)fifo_buff_ptr, &hdr, FIFO_PKT_SIZE(header));

	// Copy the remaining RPC message on the FIFO message buffer
	::read(rpc_fifo_fd,
			((fifo_buff_ptr)+FIFO_PKT_SIZE(header)),
			hdr.fifo_msg_size-FIFO_PKT_SIZE(header));

	// Return a pointer to the new message
	msg = (rpc_msg_ptr_t)(fifo_buff_ptr+hdr.rpc_msg_offset);
	// TODO provide a HEXDUMP logging routine
	//logger->Debug("FIFO RPC: RX[buff: %s]", (char*)msg);

	return (hdr.fifo_msg_size-hdr.rpc_msg_offset);
}

RPCChannelIF::plugin_data_t FifoRPC::GetPluginData(
		rpc_msg_ptr_t & msg) {
	fifo_data_t * pd;
	fs::path fifo_path(conf_fifo_dir);
	boost::system::error_code ec;
	br::rpc_fifo_app_pair_t * hdr;


	// We should have the FIFO dir already on place
	assert(initialized);
	// We should also have a valid RPC message
	assert(msg->typ==br::RPC_APP_PAIR);
	hdr = (br::rpc_fifo_app_pair_t*)((int8_t*)msg-FIFO_PKT_SIZE(app_pair));

	logger->Debug("FIFO RPC: plugin data initialization...");

	// Build a new set of plugins data
	pd = (fifo_data_t*)::malloc(sizeof(fifo_data_t));
	if (!pd) {
		logger->Error("FIFO RPC: get plugin data (malloc) FAILED");
		goto err_malloc;
	}

	// Build fifo path
	fifo_path /= "/";
	fifo_path /= hdr->rpc_fifo;

	// The application should build the channel, this could be used as
	// an additional handshaking protocol and API versioning verification
	logger->Debug("FIFO RPC: checking for application FIFO [%s]...",
			fifo_path.string().c_str());
	if (!fs::exists(fifo_path, ec)) {
		logger->Error("FIFO RPC: apps FIFO NOT FOUND [%s]...",
				fifo_path.string().c_str());
		goto err_use;
	}

	// Ensuring we have a pipe
	if (fs::status(fifo_path, ec).type() != fs::fifo_file) {
		logger->Error("FIFO RPC: apps FIFO not valid [%s]",
				fifo_path.string().c_str());
		goto err_use;
	}

	// Opening the application side pipe WRITE only
	logger->Debug("FIFO RPC: opening (WR only)...");
	pd->app_fifo_fd = ::open(fifo_path.string().c_str(), O_WRONLY);
	if (pd->app_fifo_fd < 0) {
		logger->Error("FAILED opening application RPC FIFO [%s]",
					fifo_path.string().c_str());
		pd->app_fifo_fd = 0;
		goto err_use;
	}
	::memcpy(pd->app_fifo_filename, hdr->rpc_fifo, BBQUE_FIFO_NAME_LENGTH);

	// Setting the application as initialized
	logger->Info("FIFO RPC: app channel [%d:%s] initialization DONE",
			pd->app_fifo_fd, hdr->rpc_fifo);

	return plugin_data_t(pd);

err_use:
	::free(pd);
err_malloc:
	return plugin_data_t();

}

void FifoRPC::ReleasePluginData(plugin_data_t & pd) {
	fifo_data_t * ppd = (fifo_data_t*)pd.get();

	assert(initialized==true);
	assert(ppd && ppd->app_fifo_fd);

	logger->Info("FIFO RPC: releasing app channel [%d:%s]",
			ppd->app_fifo_fd, ppd->app_fifo_filename);

	// Close the FIFO and cleanup plugin data
	::close(ppd->app_fifo_fd);

}

size_t FifoRPC::SendMessage(plugin_data_t & pd, rpc_msg_ptr_t msg,
		size_t count) {
	fifo_data_t * ppd = (fifo_data_t*)pd.get();
	br::rpc_fifo_header_t hdr;
	ssize_t error;

	assert(rpc_fifo_fd);
	assert(ppd && ppd->app_fifo_fd);

	logger->Debug("FIFO RPC: message send [typ: %d, sze: %d] "
			"using app channel [%d:%s]...",
			msg->typ, count,
			ppd->app_fifo_fd,
			ppd->app_fifo_filename);

	// Send the RPC FIFO header
	hdr.fifo_msg_size = count + FIFO_PKT_SIZE(header);
	hdr.rpc_msg_offset = count;
	hdr.rpc_msg_type = msg->typ;
	error = ::write(ppd->app_fifo_fd, (void*)&hdr, FIFO_PKT_SIZE(header));
	if (error==-1) {
		logger->Error("FIFO RPC: send massage (header) FAILED (Error %d: %s)",
				errno, strerror(errno));
		return -1;
	}

	// FIXME rollback or handle partially send (only header) messages

	// Send the RPC FIFO message payload
	error = ::write(ppd->app_fifo_fd, (void*)msg, count);
	if (error==-1) {
		logger->Error("FIFO RPC: send massage (payload) FAILED (Error %d: %s)",
				errno, strerror(errno));
		return -2;
	}

	return hdr.fifo_msg_size;
}

void FifoRPC::FreeMessage(rpc_msg_ptr_t & msg) {
	size_t hdr_size;

	// Recover the beginning of the FIFO message
	switch (msg->typ) {
	case br::RPC_APP_PAIR:
		hdr_size = FIFO_PKT_SIZE(app_pair);
		break;
	default:
		hdr_size = FIFO_PKT_SIZE(undef);
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
