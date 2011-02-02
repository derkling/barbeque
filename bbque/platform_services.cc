/**
 *       @file  platform_services.h
 *      @brief  The services provide to plugins by the barbeque core
 *
 * This class provides a set of services to barbeuqe modules throughout the
 * single InvokceServices method.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
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

#include "bbque/platform_services.h"

namespace bbque {

int32_t PlatformServices::ServiceDispatcher(const char * service_name,
		void * service_params) {
	return 0;
}

} // namespace bbque

