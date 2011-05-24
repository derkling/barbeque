/**
 *       @file  rpc_fifo.cc
 *      @brief  A message passing based RPC framework based on UNIX FIFO
 *
 * Definition of the RPC protocol based on UNIX FIFOs to implement the
 * Barbeque communication channel. This defines the communication protocol in
 * terms of message format and functionalities.
 * The communication protocol must be aligend with the RTLib supported
 * services.
 *
 * @see bbque/rtlib.h
 * @see bbque/rtlib/rpc_messages.h
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/13/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "bbque/rtlib/rpc_fifo_client.h"

#include "bbque/rtlib/rpc_messages.h"
#include "bbque/utils/utility.h"

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>


#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "RTLIB_FIFO [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "RTLIB_FIFO [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "RTLIB_FIFO [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "RTLIB_FIFO [ERR]", fmt)

namespace bbque { namespace rtlib {

BbqueRPC_FIFO_Client::BbqueRPC_FIFO_Client() :
	BbqueRPC(),
	app_fifo_path(BBQUE_PUBLIC_FIFO_PATH"/"),
	bbque_fifo_path(BBQUE_PUBLIC_FIFO_PATH"/"BBQUE_PUBLIC_FIFO) {

	DB(fprintf(stderr, FMT_DBG("Building FIFO RPC channel\n")));
}

BbqueRPC_FIFO_Client::~BbqueRPC_FIFO_Client() {
	DB(fprintf(stderr, FMT_DBG("BbqueRPC_FIFO_Client dtor\n")));
	ChannelRelease();
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::ChannelRelease() {
	rpc_fifo_undef_t fifo_undef = {
		{
			FIFO_PKT_SIZE(undef)+RPC_PKT_SIZE(app_exit),
			FIFO_PKT_SIZE(undef),
			RPC_APP_EXIT
		}
	};
	rpc_msg_app_exit_t msg_exit = {
		RPC_APP_EXIT, chTrdPid, 0};
	size_t bytes;
	int error;

	DB(fprintf(stderr, FMT_DBG("Releasing FIFO RPC channel\n")));

	// Send FIFO header
	DB(fprintf(stderr, FMT_DBG("Sending FIFO header "
		"[sze: %hd, off: %hd, typ: %hd]...\n"),
		fifo_undef.header.fifo_msg_size,
		fifo_undef.header.rpc_msg_offset,
		fifo_undef.header.rpc_msg_type
	));
	bytes = ::write(server_fifo_fd, (void*)&fifo_undef, FIFO_PKT_SIZE(undef));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s]\n"),
			bbque_fifo_path.c_str());
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	// Send RPC header
	DB(fprintf(stderr, FMT_DBG("Sending RPC header "
		"[typ: %d, pid: %d, eid: %hd]...\n"),
		msg_exit.typ,
		msg_exit.app_pid,
		msg_exit.exc_id
	));
	bytes = ::write(server_fifo_fd, (void*)&msg_exit, RPC_PKT_SIZE(app_exit));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s]\n"),
			bbque_fifo_path.c_str());
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	// Closing the private FIFO
	error = ::unlink(app_fifo_path.c_str());
	if (error) {
		fprintf(stderr, FMT_ERR("FAILED unlinking the application FIFO [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(), errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_TEARDOWN_FAILED;
	}

	return RTLIB_OK;

}

RTLIB_ExitCode BbqueRPC_FIFO_Client::WaitBbqueResp(int ms) {
	int nfds;

	nfds = epoll_wait(epoll_fd, epoll_evts, MAX_EPOLL_EVENTS, ms);
	if (nfds < 0) {
		fprintf(stderr, FMT_ERR("FAILED epoll_wait (Error %d: %s)\n"),
			errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}
	if (!nfds) {
		fprintf(stderr, FMT_DBG("TIMEOUT epoll_wait\n"));
		// TODO instead of returing an error, maybe better to try again a
		// connection by re-sending the PAIR message without wasting time on
		// destrying-reconstructing the client channel.
		// But this should be somehow controller by the application...
		return RTLIB_BBQUE_CHANNEL_READ_TIMEOUT;
	}

	// NOTE we use epoll just to monitor the client_fifo_fs, thus if we get
	// here there sould be some data ready to read
	assert(epoll_evts[0].data.fd == client_fifo_fd);

	return RTLIB_OK;
}


RTLIB_ExitCode BbqueRPC_FIFO_Client::BbqueResult() {
	rpc_fifo_header_t hdr;
	rpc_msg_resp_t resp;
	size_t bytes;

	// Read response FIFO header
	bytes = ::read(client_fifo_fd, (void*)&hdr, FIFO_PKT_SIZE(header));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}
	assert(hdr.rpc_msg_type == RPC_BBQ_RESP);

	// Read response RPC header
	bytes = ::read(client_fifo_fd, (void*)&resp, RPC_PKT_SIZE(resp));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("FAILED read from app fifo [%s] "
					"(Error %d: %s)\n"),
				app_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_READ_FAILED;
	}

	return resp.result;

}

RTLIB_ExitCode BbqueRPC_FIFO_Client::ChannelPair(const char *name) {
	rpc_fifo_app_pair_t fifo_pair = {
		{
			FIFO_PKT_SIZE(app_pair)+RPC_PKT_SIZE(app_pair),
			FIFO_PKT_SIZE(app_pair),
			RPC_APP_PAIR
		},
		"\0"
	};
	rpc_msg_app_pair_t msg_pair = {
		{RPC_APP_PAIR, chTrdPid, 0},
		BBQUE_RPC_FIFO_MAJOR_VERSION,
		BBQUE_RPC_FIFO_MINOR_VERSION,
		"\0"};
	RTLIB_ExitCode result;
	size_t bytes;

	DB(fprintf(stderr, FMT_DBG("Pairing FIFO channels [app: %s, pid: %d]\n"),
					name, chTrdPid));

	// Setting up FIFO name
	strncpy(fifo_pair.rpc_fifo, app_fifo_filename, BBQUE_FIFO_NAME_LENGTH);

	// Send FIFO header
	DB(fprintf(stderr, FMT_DBG("Sending FIFO header "
		"[sze: %hd, off: %hd, typ: %hd, pipe: %s]...\n"),
		fifo_pair.header.fifo_msg_size,
		fifo_pair.header.rpc_msg_offset,
		fifo_pair.header.rpc_msg_type,
		fifo_pair.rpc_fifo
	));
	bytes = ::write(server_fifo_fd, (void*)&fifo_pair, FIFO_PKT_SIZE(app_pair));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s]\n"),
			bbque_fifo_path.c_str());
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	::strncpy(msg_pair.app_name, name, RTLIB_APP_NAME_LENGTH);

	// Send RPC header
	DB(fprintf(stderr, FMT_DBG("Sending RPC header "
		"[typ: %d, pid: %d, eid: %hd, mjr: %d, mnr: %d, name: %s]...\n"),
		msg_pair.header.typ,
		msg_pair.header.app_pid,
		msg_pair.header.exc_id,
		msg_pair.mjr_version,
		msg_pair.mnr_version,
		msg_pair.app_name
	));
	bytes = ::write(server_fifo_fd, (void*)&msg_pair, RPC_PKT_SIZE(app_pair));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s]\n"),
				bbque_fifo_path.c_str());
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	// Waiting for BBQUE response (up to a 500ms timeout)
	result = WaitBbqueResp();
	if (result != RTLIB_OK)
		return result;

	// Check RPC server response
	result = BbqueResult();
	if (result != RTLIB_OK) {
		fprintf(stderr, FMT_ERR("bbque RPC pairing FAILED\n"));
		return result;
	}

	return RTLIB_OK;

}

RTLIB_ExitCode BbqueRPC_FIFO_Client::ChannelSetup(const char *name) {
	RTLIB_ExitCode result = RTLIB_OK;
	int error;

	DB(fprintf(stderr, FMT_INF("Initializing RPC FIFO channel\n")));

	// Opening server FIFO
	DB(fprintf(stderr, FMT_DBG("Opening bbque fifo [%s]...\n"),
				bbque_fifo_path.c_str()));
	server_fifo_fd = ::open(bbque_fifo_path.c_str(), O_WRONLY|O_NONBLOCK);
	if (server_fifo_fd < 0) {
		fprintf(stderr, FMT_ERR("FAILED opening bbque fifo [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(), errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
	}

	// Linking the server pipe to an ASIO stream descriptor
	//out.assign(server_fifo_fd);

	// Setting up application FIFO complete path
	app_fifo_path += app_fifo_filename;

	DB(fprintf(stderr, FMT_DBG("Creating [%s]...\n"),
				app_fifo_path.c_str()));

	// Creating the client side pipe
	error = ::mkfifo(app_fifo_path.c_str(), 0644);
	if (error) {
		fprintf(stderr, FMT_ERR("FAILED creating application FIFO [%s]\n"),
				app_fifo_path.c_str());
		result = RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
		goto err_create;
	}

	DB(fprintf(stderr, FMT_DBG("Opening R/W...\n")));

	// Opening the client side pipe
	// NOTE: this is opened R/W to keep it opened even if server
	// should disconnect
	client_fifo_fd = ::open(app_fifo_path.c_str(), O_RDWR);
	if (client_fifo_fd < 0) {
		fprintf(stderr, FMT_ERR("FAILED opening application FIFO [%s]\n"),
				app_fifo_path.c_str());
		result = RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
		goto err_open;
	}

	// Create epoll events used for asynchronous I/O
	epoll_fd = epoll_create(1);
	if (epoll_fd == -1) {
		fprintf(stderr, FMT_ERR("FAILED epoll creation\n"));
		result = RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
		goto err_epoll;
	}

	// Configuring epoll
	epoll_ev.events = EPOLLIN|EPOLLPRI;
	epoll_ev.data.fd = client_fifo_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fifo_fd, &epoll_ev) == -1) {
		fprintf(stderr, FMT_ERR("FAILED epoll configuration "
					"(Error %d: %s)\n"),
				errno, strerror(errno));
		result = RTLIB_BBQUE_CHANNEL_SETUP_FAILED;
		goto err_epoll;
	}

	// Pairing channel with server
	result = ChannelPair(name);
	if (result != RTLIB_OK)
		goto err_use;

	return RTLIB_OK;

err_use:
err_epoll:
	if (epoll_fd>=0)
		::close(epoll_fd);
	if (client_fifo_fd>=0)
		::close(client_fifo_fd);
err_open:
	::unlink(app_fifo_path.c_str());
err_create:
	::close(server_fifo_fd);
	return result;

}

void BbqueRPC_FIFO_Client::ChannelTrd() {
	std::unique_lock<std::mutex> chSetup_ul(chSetup_mtx);

	// Getting client PID
	chTrdPid = gettid();
	DB(fprintf(stderr, FMT_INF("RPC FIFO channel thread [PID: %d] started\n"),
				chTrdPid));

	// Notifying the thread has beed started
	trdStarted_cv.notify_one();

	// Waiting for channel setup to be completed
	chSetup_cv.wait(chSetup_ul);

	// Wait for BBQ messages

}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Init(
			const char *name) {
	std::unique_lock<std::mutex> trdStatus_ul(trdStatus_mtx);
	RTLIB_ExitCode result = RTLIB_OK;

	// Starting the communication thread
	ChTrd = std::thread(&BbqueRPC_FIFO_Client::ChannelTrd, this);
	trdStarted_cv.wait(trdStatus_ul);

	// Setting up application FIFO filename
	snprintf(app_fifo_filename, BBQUE_FIFO_NAME_LENGTH,
			"bbque_%05d_%s", chTrdPid, name);

	// Setting up the communication channel
	result = ChannelSetup(name);
	if (result != RTLIB_OK)
		return result;

	// Start the reception thread
	chSetup_cv.notify_one();

	return result;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Register(pregExCtx_t pregExCtx) {
	rpc_fifo_undef_t fifo_register = {
		{
			FIFO_PKT_SIZE(undef)+RPC_PKT_SIZE(exc_register),
			FIFO_PKT_SIZE(undef),
			RPC_EXC_REGISTER
		}
	};
	rpc_msg_exc_register_t msg_register = {
		{RPC_EXC_REGISTER, chTrdPid, pregExCtx->exc_id},
		"\0",
		"\0"
	};
	RTLIB_ExitCode result;
	size_t bytes;

	// Initializing RPC message
	::strncpy(msg_register.exc_name, pregExCtx->name.c_str(),
			RTLIB_EXC_NAME_LENGTH);
	::strncpy(msg_register.recipe, pregExCtx->exc_params.recipe,
			RTLIB_EXC_NAME_LENGTH);

	DB(fprintf(stderr, FMT_DBG("Registering EXC [%d:%d:%s]...\n"),
				msg_register.header.app_pid,
				msg_register.header.exc_id,
				msg_register.exc_name));

	// Send FIFO header
	DB(fprintf(stderr, FMT_DBG("Sending FIFO header "
		"[sze: %hd, off: %hd, typ: %hd]...\n"),
		fifo_register.header.fifo_msg_size,
		fifo_register.header.rpc_msg_offset,
		fifo_register.header.rpc_msg_type
	));
	bytes = ::write(server_fifo_fd, (void*)&fifo_register,
			FIFO_PKT_SIZE(undef));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	// Send RPC header
	DB(fprintf(stderr, FMT_DBG("Sending RPC header "
		"[typ: %d, pid: %d, eid: %hd, exc: %s, recipe: %s]...\n"),
		msg_register.header.typ,
		msg_register.header.app_pid,
		msg_register.header.exc_id,
		msg_register.exc_name,
		msg_register.recipe
	));
	bytes = ::write(server_fifo_fd, (void*)&msg_register,
			RPC_PKT_SIZE(exc_register));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	// Waiting for BBQUE response (up to a 500ms timeout)
	result = WaitBbqueResp();
	if (result != RTLIB_OK)
		return result;

	// Check RPC server response
	result = BbqueResult();
	if (result != RTLIB_OK) {
		fprintf(stderr, FMT_ERR("Execution Context registration "
					"FAILED\n"));
		return result;
	}

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Unregister(pregExCtx_t prec) {
	rpc_fifo_undef_t fifo_unregister = {
		{
			FIFO_PKT_SIZE(undef)+RPC_PKT_SIZE(exc_unregister),
			FIFO_PKT_SIZE(undef),
			RPC_EXC_UNREGISTER
		}
	};
	rpc_msg_exc_unregister_t msg_unregister = {
		{RPC_EXC_UNREGISTER, chTrdPid, prec->exc_id},
		"\0"
	};
	RTLIB_ExitCode result;
	size_t bytes;

	// Initializing RPC message
	::strncpy(msg_unregister.exc_name, prec->name.c_str(),
			RTLIB_EXC_NAME_LENGTH);

	DB(fprintf(stderr, FMT_DBG("Unregistering EXC [%d:%d:%s]...\n"),
				msg_unregister.header.app_pid,
				msg_unregister.header.exc_id,
				msg_unregister.exc_name));

	// Send FIFO header
	DB(fprintf(stderr, FMT_DBG("Sending FIFO header "
		"[sze: %hd, off: %hd, typ: %hd]...\n"),
		fifo_unregister.header.fifo_msg_size,
		fifo_unregister.header.rpc_msg_offset,
		fifo_unregister.header.rpc_msg_type
	));
	bytes = ::write(server_fifo_fd, (void*)&fifo_unregister,
			FIFO_PKT_SIZE(undef));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	// Send RPC header
	DB(fprintf(stderr, FMT_DBG("Sending RPC header "
		"[typ: %d, pid: %d, eid: %hd, exc: %s]...\n"),
		msg_unregister.header.typ,
		msg_unregister.header.app_pid,
		msg_unregister.header.exc_id,
		msg_unregister.exc_name
	));
	bytes = ::write(server_fifo_fd, (void*)&msg_unregister,
			RPC_PKT_SIZE(exc_unregister));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	// Waiting for BBQUE response (up to a 500ms timeout)
	result = WaitBbqueResp();
	if (result != RTLIB_OK)
		return result;

	// Check RPC server response
	result = BbqueResult();
	if (result != RTLIB_OK) {
		fprintf(stderr, FMT_ERR("Execution Context registration "
					"FAILED\n"));
		return result;
	}

	return RTLIB_OK;

}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Start(pregExCtx_t prec) {
	rpc_fifo_undef_t fifo_start = {
		{
			FIFO_PKT_SIZE(undef)+RPC_PKT_SIZE(exc_start),
			FIFO_PKT_SIZE(undef),
			RPC_EXC_START
		}
	};
	rpc_msg_exc_start_t msg_start = {
		{RPC_EXC_START, chTrdPid, prec->exc_id}
	};
	RTLIB_ExitCode result;
	size_t bytes;

	DB(fprintf(stderr, FMT_DBG("Starting EXC [%d:%d]...\n"),
				msg_start.header.app_pid,
				msg_start.header.exc_id));

	// Send FIFO header
	DB(fprintf(stderr, FMT_DBG("Sending FIFO header "
		"[sze: %hd, off: %hd, typ: %hd]...\n"),
		fifo_start.header.fifo_msg_size,
		fifo_start.header.rpc_msg_offset,
		fifo_start.header.rpc_msg_type
	));
	bytes = ::write(server_fifo_fd, (void*)&fifo_start,
			FIFO_PKT_SIZE(undef));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	// Send RPC header
	DB(fprintf(stderr, FMT_DBG("Sending RPC header "
		"[typ: %d, pid: %d, eid: %hd]...\n"),
		msg_start.header.typ,
		msg_start.header.app_pid,
		msg_start.header.exc_id
	));
	bytes = ::write(server_fifo_fd, (void*)&msg_start,
			RPC_PKT_SIZE(exc_start));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	// Waiting for BBQUE response (up to a 500ms timeout)
	result = WaitBbqueResp();
	if (result != RTLIB_OK)
		return result;

	// Check RPC server response
	result = BbqueResult();
	if (result != RTLIB_OK) {
		fprintf(stderr, FMT_ERR("Execution Context START FAILED\n"));
		return result;
	}

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Stop(pregExCtx_t prec) {
	rpc_fifo_undef_t fifo_stop = {
		{
			FIFO_PKT_SIZE(undef)+RPC_PKT_SIZE(exc_stop),
			FIFO_PKT_SIZE(undef),
			RPC_EXC_STOP
		}
	};
	rpc_msg_exc_stop_t msg_stop = {
		{RPC_EXC_STOP, chTrdPid, prec->exc_id}
	};
	RTLIB_ExitCode result;
	size_t bytes;

	DB(fprintf(stderr, FMT_DBG("Stopping EXC [%d:%d]...\n"),
				msg_stop.header.app_pid,
				msg_stop.header.exc_id));

	// Send FIFO header
	DB(fprintf(stderr, FMT_DBG("Sending FIFO header "
		"[sze: %hd, off: %hd, typ: %hd]...\n"),
		fifo_stop.header.fifo_msg_size,
		fifo_stop.header.rpc_msg_offset,
		fifo_stop.header.rpc_msg_type
	));
	bytes = ::write(server_fifo_fd, (void*)&fifo_stop,
			FIFO_PKT_SIZE(undef));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	// Send RPC header
	DB(fprintf(stderr, FMT_DBG("Sending RPC header "
		"[typ: %d, pid: %d, eid: %hd]...\n"),
		msg_stop.header.typ,
		msg_stop.header.app_pid,
		msg_stop.header.exc_id
	));
	bytes = ::write(server_fifo_fd, (void*)&msg_stop,
			RPC_PKT_SIZE(exc_stop));
	if (bytes<=0) {
		fprintf(stderr, FMT_ERR("write to BBQUE fifo FAILED [%s] "
					"(Error %d: %s)\n"),
				bbque_fifo_path.c_str(),
				errno, strerror(errno));
		return RTLIB_BBQUE_CHANNEL_WRITE_FAILED;
	}

	DB(fprintf(stderr, FMT_DBG("Waiting BBQUE response...\n")));

	// Waiting for BBQUE response (up to a 500ms timeout)
	result = WaitBbqueResp();
	if (result != RTLIB_OK)
		return result;

	// Check RPC server response
	result = BbqueResult();
	if (result != RTLIB_OK) {
		fprintf(stderr, FMT_ERR("Execution Context STOP FAILED\n"));
		return result;
	}

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Set(
			const RTLIB_ExecutionContextHandler ech,
			RTLIB_Constraint* constraints,
			uint8_t count) {
	//Silence "args not used" warning.
	(void)ech;
	(void)constraints;
	(void)count;

	fprintf(stderr, FMT_WRN("EXC Set: not yet implemeted\n"));

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_Clear(
			const RTLIB_ExecutionContextHandler ech) {
	//Silence "args not used" warning.
	(void)ech;

	fprintf(stderr, FMT_WRN("EXC Clear: not yet implemeted\n"));

	return RTLIB_OK;
}

RTLIB_ExitCode BbqueRPC_FIFO_Client::_GetWorkingMode(
		pregExCtx_t prec,
		RTLIB_WorkingModeParams *wm) {
	//Silence "args not used" warning.
	(void)prec;
	(void)wm;

	fprintf(stderr, FMT_DBG("EXC GetWorkingMode: not yet implemeted"));

	return RTLIB_OK;
}

void BbqueRPC_FIFO_Client::_Exit() {
	ChannelRelease();
}

} // namespace rtlib

} // namespace bbque

