/**
 *       @file  plugin.h
 *      @brief  The generic plugin interface for the Barbeque RTRM
 *
 * This provides a generic interface (C/C++) for Barbeque plugins.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
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
 * =====================================================================================
 */

#ifndef BBQUE_PLUGIN_H_
#define BBQUE_PLUGIN_H_

#include <stdint.h>

#include "bbque/platform_services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The plugin programming language
 */
typedef enum PF_ProgrammingLanguage {
	/** Undefined plugin language */
	PF_LANG_UNDEF = 0,
	/** Plugin coded in C language */
	PF_LANG_C,
	/** Plugin coded in CPP language */
	PF_LANG_CPP
} PF_ProgrammingLanguage;

struct PF_PlatformServices;

/**
 * The information passed to a plugin to support the construction of a new
 * registered object
 */
typedef struct PF_ObjectParams {
	/** The name of the object to built */
	const char * id;
	/** The set of platform supported services */
	const struct PF_PlatformServices * platform_services;
	/** A pointer to module specific data */
	void * data;
} PF_ObjectParams;

/**
 * The API version number
 */
typedef struct PF_PluginAPIVersion {
	/** Major API version number */
	int32_t major;
	/** Minor API version number */
	int32_t minor;
} PF_PluginAPIVersion;

/**
 * @brief A plugin provided function to build a new object
 * This function allow the plugin manager to create a new plugin object. Each
 * plugin is required to registers such a functions with the plugin manager at
 * initialization time.
 */
typedef void * (*PF_CreateFunc)(PF_ObjectParams *);

/**
 * @brief A plugin provided function to destroy a previously created object
 * This function allow the plugin manager to destroy a specified plugin object. Each
 * plugin is required to registers such a functions with the plugin manager at
 * initialization time.
 */
typedef int32_t (*PF_DestroyFunc)(void *);

/**
 * Contains all the information that a plugin must provide to the plugin
 * manager upon initialization (e.g. version, create/destroy functions, and
 * programming language).
 */
typedef struct PF_RegisterParams {
	/** The plugins implemented API version */
	PF_PluginAPIVersion version;
	/** The plugin code language */
	PF_ProgrammingLanguage programming_language;
	/** The plugins object creation function */
	PF_CreateFunc CreateFunc;
	/** The plugins object destruction function */
	PF_DestroyFunc DestroyFunc;
} PF_RegisterParams;

/**
 * @brief A pointer to an object registration function.
 * A function implemented by the plugin manager which allows each plugin
 * to register a PF_RegisterParams struct for each object type it supports.
 * @note This scheme allows a plugin to register different versions of an
 * object and multiple object types.
 */
typedef int32_t (*PF_RegisterFunc)(const char * node_type,
					const PF_RegisterParams * params);

/**
 * @brief A pointer to a service invocation function.
 * A pointer to a generic function that plugins can use to invoke services of
 * the Barbeque core (e.g. configuration parameters, logging, event
 * notification and error reporting) The signature includes the service name
 * and an opaque pointer to a parameters struct. The plugins should know about
 * available services and how to invoke them.
 */
typedef int32_t (*PF_InvokeServiceFunc)(PF_PlatformServiceID id,
					PF_ServiceData & data);
/**
 * @brief Information passed to plugins at initialization time
 * This struct aggregate all the services that the platform provides to plugin
 * (e.g., version, registering objects and the invoke service function). This
 * struct is passed to each plugin at initialization time.
 */
typedef struct PF_PlatformServices {
	/** Current version of the plugins API */
	PF_PluginAPIVersion version;
	/** Plugins objects registration function
	  Plugins could use this function at initialization time to register each
	  object they want. */
	PF_RegisterFunc RegisterObject;
	/** Plugins service invocation function
	  Plugins could access platform offered services by calls to this function
	  */
	PF_InvokeServiceFunc InvokeService;
} PF_PlatformServices;


/**
 * @brief A pointer to a plugin exit function
 * Defines a pointer to a plugin define exit function.
 */
typedef int32_t (*PF_ExitFunc)();


/**
 * @brief The plugin initialization function pointer
 *
 * Type definition of the PF_initPlugin function bellow (used by
 * PluginManager to initialize plugins) Note the return type is the
 * PF_ExitFunc (used by PluginManager to tell plugins to cleanup). If the
 * initialization failed for any reason the plugin may report the error via
 * the error reporting function of the provided platform services.
 * Nevertheless, it must return NULL exit func in this case to let the plugin
 * manger that the plugin wasn't initialized properly. The plugin may use the
 * runtime services - allocate memory, log messages and of course register
 * node types.
 *
 * @param  params the platform services struct
 * @return the exit func of the plugin or NULL if initialization failed.
 */
typedef PF_ExitFunc (*PF_InitFunc)(const PF_PlatformServices *);


/**
 * @brief Named exported entry point into the plugin
 *
 * This definition is required eventhough the function is loaded from a
 * dynamic library by name and casted to PF_InitFunc. If this declaration is
 * commented out DynamicLibrary::getSymbol() fails
 *
 * The plugin's initialization function MUST be called "PF_initPlugin" (and
 * conform to the signature of course).
 *
 * @param  params the platform services struct
 * @return the exit func of the plugin or NULL if initialization failed.
 */
#ifdef  __cplusplus
extern "C" PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params);
#else
extern PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params);
#endif


/**
 * @brief The data structure collecting exported plugins methods.
 *
 * Basically this data structure is used to export in a "compiler friendly"
 * way the plugins entry point. Indeed, due to a weakness of the ISO C standard,
 * there is no valid cast between pointer to function and pointer to objects,
 * thus a dlsym cast to a punction pointer will result (at least) on a
 * compiler warning.<br>
 * For more details, see the rationale section of the dlsym manpages.
 */
typedef struct PF_ExportedSymbols {
	/** The plugin entry point */
	PF_InitFunc init;
} PF_ExportedSymbols_t;

#define PLUGIN_SYMBOL_TABLE "PF_exportedSymbols"
#define PLUGIN_INIT(FUNC)\
	PF_ExportedSymbols_t PF_exportedSymbols = {FUNC}

#ifdef  __cplusplus
}
#endif

#endif // BBQUE_PLUGIN_H_

