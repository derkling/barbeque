/**
 *       @file  linux.h
 *      @brief  A platform proxy to control resources on a Linux host.
 *
 * This class provides the platform specific methods of the PlatformProxy
 * class which are suitable for controlling resources of a Linux system. This
 * implementation exploits Control Groups, specifically the cpuset subsystem,
 * to assign CPU and MEMORY resources to Barbeque controlled resources.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  11/23/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#ifndef BBQUE_LINUX_PP_H_
#define BBQUE_LINUX_PP_H_

#include "bbque/platform_proxy.h"
#include "bbque/utils/attributes_container.h"

#include <libcgroup.h>

/**
 * @brief Default MAX number of CPUs per socket
 */
#define DEFAULT_MAX_CPUS 16

/**
 * @brief Default MAX number of MEMs node per host
 */
#define DEFAULT_MAX_MEMS 16

/**
 * @brief The namespace of the Linux platform integration module
 */
#define PLAT_LNX_ATTRIBUTE PLATFORM_PROXY_NAMESPACE".lnx"

using bbque::res::ResourcePtr_t;
using bbque::utils::AttributesContainer;

namespace bbque {

/**
 * @brief The Linux Platform Proxy module
 */
class LinuxPP : public PlatformProxy {

public:


	struct CGroupPtrDlt {
		CGroupPtrDlt(void) {}
		void operator()(struct cgroup *ptr) const {
			cgroup_free(&ptr);
		}
	};


	LinuxPP();

	virtual ~LinuxPP();

private:

	typedef enum RLinuxType {
		RLINUX_TYPE_NODE = 0,	/* A Host Linux machine */
		RLINUX_TYPE_SOCKET,		/* A socket on an SMP Linux machine */
		RLINUX_TYPE_CPU,		/* A CPU on an SMP Linux machine */
		RLINUX_TYPE_SMEM,		/* A socket memory bank on an SMP Linux
								   machine */

		RLINUX_TYPE_UNKNOWN		/* Resource which could not be mapped on an Host
								   Linux machine */
	} RLinuxType_t;
	/**
	 * @breif Resource assignement bindings on a Linux machine
	 */
	typedef struct RLinuxBindings {
		unsigned short node_id; ///> Maps a "tile" on Host Linux machines
		unsigned short socket_id; ///> Maps a "cluster" on SMP Linux machine
		char *cpus;
		char *mems;
		RLinuxBindings(const uint8_t MaxCpusCount, const uint8_t MaxMemsCount) :
			cpus(NULL), mems(NULL) {
			// 3 chars are required for each CPU/MEM resource if formatted
			// with syntax: "nn,". This allows for up-to 99 resources per
			// cluster
			if (MaxCpusCount)
				cpus = (char*)calloc(3*MaxCpusCount, sizeof(char));
			if (MaxMemsCount)
				mems = (char*)calloc(3*MaxMemsCount, sizeof(char));
		}
		~RLinuxBindings() {
			free(cpus);
			free(mems);
		}
	} RLinuxBindings_t;

	typedef std::shared_ptr<RLinuxBindings_t> RLinuxBindingsPtr_t;

	typedef struct CGroupData : public AttributesContainer::Attribute {
		AppPtr_t papp; /** The controlled application */
		char cgpath[20]; // "bbq/12345:ABCDEF:00";
		struct cgroup *pcg;
		struct cgroup_controller *pc_cpuset;

		CGroupData(AppPtr_t pa) :
			Attribute(PLAT_LNX_ATTRIBUTE, "cgroup"),
			papp(pa), pcg(NULL), pc_cpuset(NULL) {
			snprintf(cgpath, 20, "bbq/%s", papp->StrId());
		}

		CGroupData(const char *cgp) :
			Attribute(PLAT_LNX_ATTRIBUTE, "cgroup"),
			pcg(NULL), pc_cpuset(NULL) {
			snprintf(cgpath, 20, "%s", cgp);
		}

		~CGroupData() {
			// Removing Kernel Control Group
			cgroup_delete_cgroup(pcg, 1);
			// Releasing libcgroup resources
			cgroup_free(&pcg);
		}

	} CGroupData_t;

	typedef std::shared_ptr<CGroupData_t> CGroupDataPtr_t;

	/**
	 * @brief the control group controller
	 *
	 * This is a reference to the controller used on a generic Linux host.
	 * So far we use the "cpuset" controller.
	 */
	const char *controller;

	/**
	 * @brief The maximum number of CPUs per socket
	 */
	uint8_t MaxCpusCount;

	/**
	 * @brief The maximum number of MEMs per node
	 */
	uint8_t MaxMemsCount;

	/**
	 * @brief The "silos" CGroup
	 *
	 * The "silos" is a control group where are placed processe which have
	 * been scheduled. This CGroup is indended to be a resource constrained
	 * group which grants a bare minimun of resources for the controlling
	 * application to run the RTLib
	 */
	CGroupDataPtr_t psilos;

/**
 * @defgroup group_plt_prx Platform Proxy
 * @{
 * @name Linux Platform Proxy
 *
 * The linux platform proxy is provided to control resources of a genric
 * host machine running on top of a Linux kernel, version 2.6.24 or higher.
 *
 * For a comprensive introduction to resopurce management using Control Groups
 * please refers to the RedHat documentation available at this link:
 * http://docs.redhat.com/docs/en-US/Red_Hat_Enterprise_Linux/6/html/Resource_Management_Guide/index.html
 *
 * @name Basic Infrastructure and Initialization
 * @{
 */

	ExitCode_t RegisterCluster(RLinuxBindingsPtr_t prlb);
	ExitCode_t RegisterClusterCPUs(RLinuxBindingsPtr_t prlb);
	ExitCode_t ParseNode(struct cgroup_file_info &entry);
	ExitCode_t ParseNodeAttributes(struct cgroup_file_info &entry,
			RLinuxBindingsPtr_t prlb);

	/**
	 * @brief Parse the resources assigned to Bqrbeque by CGroup
	 *
	 * This method allows to parse a set of resources, assinged to Barbeque for
	 * run-time management, which are defined with a properly configure
	 * control group.
	 */
	ExitCode_t _LoadPlatformData();
	ExitCode_t _Setup(AppPtr_t papp);
	ExitCode_t _Release(AppPtr_t papp);

	ExitCode_t _ReclaimResources(AppPtr_t papp);

	ExitCode_t _MapResources(AppPtr_t papp, UsagesMapPtr_t pres,
		RViewToken_t rvt, bool excl);

/**
 * @}
 * @}
 */


	RLinuxType_t GetRLinuxType(ResourcePtr_t pres);

	uint8_t GetRLinuxId(ResourcePtr_t pres);

	ExitCode_t GetResouceMapping(AppPtr_t papp, UsagesMapPtr_t pum,
		RViewToken_t rvt, RLinuxBindingsPtr_t prlb);

	ExitCode_t BuildCGroup(CGroupDataPtr_t &pcgd);

	ExitCode_t BuildSilosCG(CGroupDataPtr_t &pcgd);
	ExitCode_t BuildAppCG(AppPtr_t papp, CGroupDataPtr_t &pcgd);

	ExitCode_t GetCGroupData(AppPtr_t papp, CGroupDataPtr_t &pcgd);
	ExitCode_t SetupCGroup(CGroupDataPtr_t &pcgd, RLinuxBindingsPtr_t prlb,
			bool excl = false, bool move = true);

};

} /* bbque */

#endif /* end of include guard: BBQUE_PLATFORM_PROXY_H_ */
