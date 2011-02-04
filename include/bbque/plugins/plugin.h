/**
 *       @file  plugin.h
 *      @brief  The generic plugin interface for the Barbeque RTRM
 *
 * This provides a generic interface (C/C++) for Barbeque plugins.
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
 * =====================================================================================
 */

#ifndef BBQUE_PLUGIN_H_
#define BBQUE_PLUGIN_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PF_ProgrammingLanguage {
	PF_LANG_C,
	PF_LANG_CPP
} PF_ProgrammingLanguage;

struct PF_PlatformServices;

typedef struct PF_ObjectParams {
	const char * id;
	const struct PF_PlatformServices * platform_services;
} PF_ObjectParams;

typedef struct PF_PluginAPIVersion {
	int32_t major;
	int32_t minor;
} PF_PluginAPIVersion;

typedef void * (*PF_CreateFunc)(PF_ObjectParams *);

typedef int32_t (*PF_DestroyFunc)(void *);

typedef struct PF_RegisterParams {
	PF_PluginAPIVersion version;
	PF_ProgrammingLanguage programming_language;
	PF_CreateFunc CreateFunc;
	PF_DestroyFunc DestroyFunc;
} PF_RegisterParams;

typedef int32_t (*PF_RegisterFunc)(const char * node_type,
					const PF_RegisterParams * params);

typedef int32_t (*PF_InvokeServiceFunc)(const char * service_name,
					void * service_params);

typedef struct PF_PlatformServices {
	PF_PluginAPIVersion version;
	PF_RegisterFunc RegisterObject;
	PF_InvokeServiceFunc InvokeService;
} PF_PlatformServices;

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
 * @param  [const PF_PlatformServices *] params - the platform services struct
 * @retval [PF_ExitFunc] the exit func of the plugin or NULL if initialization failed.
 */
typedef PF_ExitFunc (*PF_InitFunc)(const PF_PlatformServices *);

/**
 * @brief
 *
 * Named exported entry point into the plugin This definition is required
 * eventhough the function is loaded from a dynamic library by name and casted
 * to PF_InitFunc. If this declaration is commented out
 * DynamicLibrary::getSymbol() fails
 *
 * The plugin's initialization function MUST be called "PF_initPlugin" (and
 * conform to the signature of course).
 *
 * @param  [const PF_PlatformServices *] params - the platform services struct
 * @retval [PF_ExitFunc] the exit func of the plugin or NULL if initialization failed.
 */
#ifdef  __cplusplus
extern "C" PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params);
#else
extern PF_ExitFunc PF_initPlugin(const PF_PlatformServices * params);
#endif

#ifdef  __cplusplus
}
#endif

#endif // BBQUE_PLUGIN_H_

