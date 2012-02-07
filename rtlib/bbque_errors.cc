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

const char *RTLIB_errorStr[] = {

	//RTLIB_OK
	"Success (no errors)",
	//RTLIB_ERROR
	"Uspecified (generic) error",
	//RTLIB_VERSION_MISMATCH
	"RTLib Version does not match that of the Barbeque RTRM",
	//RTLIB_NO_WORKING_MODE
	"No new working mode error",

//---- Barbeque Communicaiton errors

	//RTLIB_BBQUE_CHANNEL_SETUP_FAILED
	"Failed to setup the channel to connect the Barbeque RTRM",
	//RTLIB_BBQUE_CHANNEL_TEARDOWN_FAILED
	"Failed to release the channel to connect the Barbeque RTRM",
	//RTLIB_BBQUE_CHANNEL_WRITE_FAILED
	"Failed to write to the Barbeque RTRM communication channel",
	//RTLIB_BBQUE_CHANNEL_READ_FAILED
	"Failed to read form the Barbeque RTRM communication channel",
	//RTLIB_BBQUE_CHANNEL_READ_TIMEOUT
	"Timeout on read form the Barbeque RTRM communication channel",
	//RTLIB_BBQUE_CHANNEL_PROTOCOL_MISMATCH
	"The bbque and application RPC protocol versions does not match",
	//RTLIB_BBQUE_CHANNEL_UNAVAILABLE
	"The (expected) communicaiton channel is not available",
	//RTLIB_BBQUE_CHANNEL_TIMEOUT
	"The (expected) response has gone in TIMEOUT",
	//RTLIB_BBQUE_UNREACHABLE
	"The Barbeque RTRM is not available",

//---- EXC Management errors

	//RTLIB_EXC_DUPLICATE
	"The Execution Context Duplicated",
	//RTLIB_EXC_NOT_REGISTERED
	"The Execution Context has not been registered",
	//RTLIB_EXC_REGISTRATION_FAILED
	"The Execution Context Registration Failed",
	//RTLIB_EXC_MISSING_RECIPE
	"The Execution Context Registration Failed",
	//RTLIB_EXC_UNREGISTRATION_FAILED
	"The Execution Context Unregistration Failed",
	//RTLIB_EXC_NOT_STARTED
	"The Execution Context has not been started yet",
	//RTLIB_EXC_ENABLE_FAILED
	"The Execution Context Start Failed",
	//RTLIB_EXC_NOT_ENABLED
	"The Execution Context is not currentyl enabled",
	//RTLIB_EXC_DISABLE_FAILED
	"The Execution Context Stop Failed",
	//RTLIB_EXC_GWM_FAILED
	"Failed to get a working mode",

//---- Reconfiguration actions required for an EXC
// NOTE these values should match (in number and order) those defined by the
//	    ApplicationStatusIF::SyncState_t

	//RTLIB_EXC_GWM_START
	"Start running on the assigned AWM",
	//RTLIB_EXC_GWM_RECONF
	"Reconfiguration into a different AWM",
	//RTLIB_EXC_GWM_MIGREC
	"Migration and reconfiguration into a different AWM",
	//RTLIB_EXC_GWM_MIGRATE
	"Migration (still running on the same AWM)",
	//RTLIB_EXC_GWM_BLOCKED
	"EXC suspended (resources not available)",

//---- Internal values not exposed to applications

	//RTLIB_EXC_SYNC_MODE
	"The EXC is in sync mode",
	//RTLIB_EXC_SYNCP_FAILED
	"A step of the synchronization protocol has failed",
	//RTLIB_EXC_WORKLOAD_NONE
	"No more workload to process",

};

