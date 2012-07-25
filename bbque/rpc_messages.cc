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

#include "bbque/rtlib/rpc_messages.h"
#include "bbque/utils/utility.h"

namespace bbque { namespace rtlib {

const char *RPC_messageStr[] = {

//--- Application Originated Messages
	//RPC_APP_PAIR
	"APair",
	//RPC_APP_EXIT
	"AExit",

	//RPC_APP_RESP
	"AResp",
	//RPC_APP_MSGS_COUNT
	"ACount",

//--- Execution Context Originated Messages
	//RPC_EXC_REGISTER
	"EReg",
	//RPC_EXC_UNREGISTER
	"EUnreg",
	//RPC_EXC_SET
	"ESet",
	//RPC_EXC_CLEAR
	"EClear",
	//RPC_EXC_GGAP
	"ENap",
	//RPC_EXC_START
	"EStrat",
	//RPC_EXC_STOP
	"EStop",
	//RPC_EXC_SCHEDULE
	"ESched",

	//RPC_EXC_RESP
	"EResp",
	//RPC_EXC_MSGS_COUNT
	"ECount",

//--- Barbeque Originated Messages
	//RPC_BBQ_STOP_EXECUTION
	"BStop",

	//RPC_BBQ_SYNCP_PRECHANGE
	"BSPrC",
	//RPC_BBQ_SYNCP_SYNCCHANGE
	"BSSyC",
	//RPC_BBQ_SYNCP_DOCHANGE
	"BSDoC",
	//RPC_BBQ_SYNCP_POSTCHANGE
	"BSPoC",

	//RPC_BBQ_RESP
	"BResp",
	//RPC_BBQ_MSGS_COUNT
	"BCount",

};

// Checking RPC messages string array consistency
static_assert(ARRAY_SIZE(RPC_messageStr) == bbque::rtlib::RPC_BBQ_MSGS_COUNT+1,
		"RPC message strings not matching messages type count");

} // namespace rtlib

} // namespace bbque

