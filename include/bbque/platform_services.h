/**
 *       @file  platform_services.h
 *      @brief  The services provided to plugins by the Barbeque framework
 *
 * This class provides a set of services to barbeuqe modules throughout the
 * single InvokeServices method.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
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


#ifndef BBQUE_PLATFORM_SERVICES_H_
#define BBQUE_PLATFORM_SERVICES_H_

/**
 * Defines the set of supported platform services.
 * Each platform service could be associated to a corresponding service params
 * data structure defined thereafter. Some values are for internal use only
 * and do not correspond to an actual offered service.
 */
typedef enum PF_PlatformServiceID {
	/** <i>(internal)</i> Used to represent the "no service" */
	PF_SERVICE_NONE = 0,

//----- Services for both C and CPP coded plugins

	/** Return the string of a configuration param */
	PF_SERVICE_CONF_PARAM,

	/** <i>(internal)</i> This must always be the
	  last entry of services for both C and CPP
	  coded plugins*/
	PF_SERVICE_C_BASED_COUNT,

//----- Services only for CPP coded plugins

	/** Return the values_map of the required configuration params */
	PF_SERVICE_CONF_DATA,

	/** <i>(internal)</i> This must always be the last entry */
	PF_SERVICE_COUNT

} PF_PlatformServiceID;

/**
 * Defines the data exchange protocol between modules and service dispatcher.
 * Modules could request services defined by the PF_PlatformServiceID enum and
 * must provide a reference to a PF_ServiceData structure. This last allows to
 * define a set of service specific input data and expected return results.
 */
typedef struct PF_ServiceData {
	/** Identifier of the requesting object */
	const char * id;
	/** Specific service request data */
	void * request;
	/** Specific service responce data */
	void * response;
} PF_ServiceData;

/**
 * Exit codes for a service request
 */
typedef enum PF_ServiceResponse {
	/** Request satisfied, response data returned */
	PF_SERVICE_DONE 	=  0,
	/** Undefined service request */
	PF_SERVICE_UNDEF	= -1,
	/** Wrong request data for the required service */
	PF_SERVICE_WRONG	= -2
} PF_ServiceResponse;


//----- START - PF_SERVICE_CONF_DATA -----
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
using boost::program_options::options_description;
using boost::program_options::variables_map;

/**
 * Request data for a PF_SERVICE_CONF_DATA service request
 */
typedef struct PF_Service_ConfDataIn {
	/** Pointer to a set of options descriptors required */
	options_description const * opts_desc;
} PF_Service_ConfDataIn;

/**
 * Response data for a PF_SERVICE_CONF_DATA service request
 */
typedef struct PF_Service_ConfDataOut {
	/** Pointer to a map to store the values of required options */
	variables_map * opts_value;
} PF_Service_ConfDataOut;
//----- END - PF_SERVICE_CONF_DATA -----



#ifdef __cplusplus

#include <cstdint>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace bbque {


/**
 * @class PlatformServices
 * @brief Module implemeting supported for offered platform services
 * This class defines the interface to access all the services that the
 * platform provides to plugin. Offered services have a unique identifyer
 * defined by PF_PlatformServiceID and requires a corresponding set of input
 * and output parameters to be passed using the PF_ServiceData structure.
 * Each service defines the structure of input and output parameters to be
 * passed at service invocation time. However, each service request return a
 * standardized return code as defined by PF_ServiceResponse.
 */
class PlatformServices {

public:

	/**
	 * @brief Release the platform service module
	 */
	~PlatformServices();

	/**
	 * @brief Get an instance to the (singleton) service module
	 */
	static PlatformServices & GetInstance();

	/**
	 * @brief Entry point for platform service requests
	 * This method could be passed to plugins as a valid instance of
	 * PF_InvokeServiceFunc. After some safety checking on the service source
	 * and type, this method dispatch the request to the proper service
	 * provider.
	 */
	static int32_t ServiceDispatcher(PF_PlatformServiceID id,
			PF_ServiceData & data);

private:

	/**
	 * @ brief Build a new platform service module.
	 */
	PlatformServices();

	/**
	 * @brief   Verify if the specified request is authorized
	 * This method could implement an ACL policy to allows some services only
	 * to some modules. For instance, there are some requests that should be
	 * asserted only by CPP coded modules.
	 * @param   id		service identifyed
	 * @return  data 	service data
	 */
	bool CheckRequest(PF_PlatformServiceID id,
		PF_ServiceData & data);

	/**
	 * @brief The PF_SERVICE_CONF_DATA service provide
	 * This method process service data requests by quering the
	 * ConfigurationManager module about the required options and than returns
	 * their values to the calling plugin.
	 */
	int32_t ServiceConfData(PF_ServiceData & data);

};

} // namespace bbque

#endif // __cplusplus

#endif // BBQUE_PLATFORM_SERVICE_H_

