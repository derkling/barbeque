/**
 *       @file  linux.cc
 *      @brief  A platform proxy to control resources on a Linux host.
 *
*
 *     @author  Patrick Bellasi (derkling), derkling@google.com
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

#include "bbque/pp/linux.h"

#include "bbque/res/resource_accounter.h"
#include "bbque/res/resource_utils.h"

#include <string.h>
#include <linux/version.h>

#define BBQUE_LINUXPP_PLATFORM_ID		"org.linux.cgroup"

#define BBQUE_LINUXPP_SILOS 			BBQUE_LINUXPP_CGROUP"/silos"

#define BBQUE_LINUXPP_CPUS_PARAM 		"cpuset.cpus"
#define BBQUE_LINUXPP_CPUP_PARAM 		"cpu.cfs_period_us"
#define BBQUE_LINUXPP_CPUQ_PARAM 		"cpu.cfs_quota_us"
#define BBQUE_LINUXPP_MEMN_PARAM 		"cpuset.mems"
#define BBQUE_LINUXPP_MEMB_PARAM 		"memory.limit_in_bytes"
#define BBQUE_LINUXPP_CPU_EXCLUSIVE_PARAM 	"cpuset.cpu_exclusive"
#define BBQUE_LINUXPP_MEM_EXCLUSIVE_PARAM 	"cpuset.mem_exclusive"
#define BBQUE_LINUXPP_PROCS_PARAM		"cgroup.procs"

// The default CFS bandwidth period [us]
#define BBQUE_LINUXPP_CPUP_DEFAULT		100000

// Checking for kernel version requirements
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
# error Linux kernel >= 3.0 required by the Platform Integration Layer
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
# warning CPU Quota management disabled, Linux kernel >= 3.2 required
#endif


namespace bb = bbque;
namespace br = bbque::res;

namespace bbque {

LinuxPP::LinuxPP() :
	PlatformProxy(),
	controller("cpuset"),
	MaxCpusCount(DEFAULT_MAX_CPUS),
	MaxMemsCount(DEFAULT_MAX_MEMS) {
	ExitCode_t pp_result = OK;
	char *mount_path = NULL;
	int cg_result;

	// Init the Control Group Library
	cg_result = cgroup_init();
	if (cg_result) {
		logger->Error("PLAT LNX: CGroup Library initializaton FAILED! "
				"(Error: %d - %s)", cg_result, cgroup_strerror(cg_result));
		return;
	}

	cg_result = cgroup_get_subsys_mount_point(controller, &mount_path);
	if (cg_result) {
		logger->Error("PLAT LNX: CGroup Library mountpoint lookup FAILED! "
				"(Error: %d - %s)", cg_result, cgroup_strerror(cg_result));
		return;
	}
	logger->Info("PLAT LNX: controller [%s] mounted at [%s]",
			controller, mount_path);


	// TODO: check that the "bbq" cgroup already existis
	// TODO: check that the "bbq" cgroup has CPUS and MEMS
	// TODO: keep track of overall associated resources => this should be done
	// by exploting the LoadPlatformData low-level method
	// TODO: update the MaxCpusCount and MaxMemsCount

	// Build "silos" CGroup to host blocked applications
	pp_result = BuildSilosCG(psilos);
	if (pp_result) {
		logger->Error("PLAT LNX: Silos CGroup setup FAILED!");
		return;
	}

	free(mount_path);

	// Mark the Platform Integration Layer (PIL) as initialized
	SetPilInitialized();
}

LinuxPP::~LinuxPP() {

}

/*******************************************************************************
 *    Platform Resources Parsing and Loading
 ******************************************************************************/

LinuxPP::ExitCode_t
LinuxPP::RegisterClusterCPUs(RLinuxBindingsPtr_t prlb) {
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	char resourcePath[] = "arch.tile0.cluster256.pe256";
	unsigned short first_cpu_id;
	unsigned short last_cpu_id;
	const char *p = prlb->cpus;
	uint32_t cpu_quota = 100;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)

	// NOTE: The CPU bandwidth is used to assign the SAME quota to each
	// processor within the same node/cluster. This is not the intended
	// behavior of the cfs_quota_us, but simplifies a lot the
	// configuration and should be just enough for our purposes.
	// Thus, each CPU will receive a % of CPU time defined by:
	//   QUOTA = CPU_QUOTA * 100 / CPU_PERIOD
	if (prlb->amount_cpup) {
		cpu_quota = (prlb->amount_cpuq * 100) / prlb->amount_cpup;
		logger->Debug("Registering CPUs of node [%d] with CPU quota of [%lu]%",
				prlb->socket_id, cpu_quota);
	}

#endif

	while (*p) {

		// Get a CPU id, and register the corresponding resource path
		sscanf(p, "%hu", &first_cpu_id);
		snprintf(resourcePath+18, 10, "%hu.pe%d",
				prlb->socket_id, first_cpu_id);
		logger->Debug("PLAT LNX: Registering [%s]...", resourcePath);
		ra.RegisterResource(resourcePath, "", cpu_quota);

		// Look-up for next CPU id
		while (*p && (*p != ',') && (*p != '-')) {
			++p;
		}

		if (!*p)
			return OK;

		if (*p == ',') {
			++p;
			continue;
		}
		// Otherwise: we have stopped on a "-"

		// Get last CPU of this range
		sscanf(++p, "%hu", &last_cpu_id);
		// Register all the other CPUs of this range
		while (++first_cpu_id <= last_cpu_id) {
			snprintf(resourcePath+18, 8, "%hu.pe%d",
					prlb->socket_id, first_cpu_id);
			logger->Debug("PLAT LNX: Registering [%s]...", resourcePath);
			ra.RegisterResource(resourcePath, "", cpu_quota);
		}

		// Look-up for next CPU id
		while (*p && (*p != ',')) {
				++p;
		}

		if (*p == ',')
			++p;
	}

	return OK;
}

LinuxPP::ExitCode_t
LinuxPP::RegisterClusterMEMs(RLinuxBindingsPtr_t prlb) {
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	char resourcePath[] = "arch.tile0.cluster256.mem256";
	uint64_t limit_in_bytes = atol(prlb->memb);

	// Setup resource path
	snprintf(resourcePath+18, 11, "%hu.mem0", prlb->socket_id);

	logger->Debug("PLAT LNX: Registering [%s: %llu Bytes]...",
			resourcePath, limit_in_bytes);
	ra.RegisterResource(resourcePath, "Bytes", limit_in_bytes);

	return OK;
}

LinuxPP::ExitCode_t
LinuxPP::RegisterCluster(RLinuxBindingsPtr_t prlb) {
	ExitCode_t pp_result = OK;

	logger->Debug("PLAT LNX: Setup resources for Node [%d], "
			"CPUs [%s], MEMs [%s]",
			prlb->socket_id, prlb->cpus, prlb->mems);

	// The CPUs are generally represented with a syntax like this:
	// 1-3,4,5-7
	pp_result = RegisterClusterCPUs(prlb);
	if (pp_result != OK)
		return pp_result;

	// The MEMORY amount is represented in Bytes
	pp_result = RegisterClusterMEMs(prlb);
	if (pp_result != OK)
		return pp_result;

	return pp_result;
}

LinuxPP::ExitCode_t
LinuxPP::ParseNodeAttributes(struct cgroup_file_info &entry,
		RLinuxBindingsPtr_t prlb) {
	char group_name[] = BBQUE_LINUXPP_RESOURCES "/" BBQUE_LINUXPP_CLUSTER "123";
	struct cgroup_controller *cg_controller = NULL;
	struct cgroup *bbq_node = NULL;
	ExitCode_t pp_result = OK;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
	char *buff = NULL;
#endif
	int cg_result;

	// Read "cpuset" attributes from kernel
	logger->Debug("PLAT LNX: Loading kernel info for [%s]...", entry.path);


	// Initialize the CGroup variable
	sscanf(entry.path + STRLEN(BBQUE_LINUXPP_CLUSTER), "%hu",
			&prlb->socket_id);
	snprintf(group_name +
			STRLEN(BBQUE_LINUXPP_RESOURCES) +   // e.g. "bbque/res"
			STRLEN(BBQUE_LINUXPP_CLUSTER) + 1,  // e.g. "/" + "node"
			4, "%d",
			prlb->socket_id);
	bbq_node = cgroup_new_cgroup(group_name);
	if (bbq_node == NULL) {
		logger->Error("PLAT LNX: Parsing resources FAILED! "
				"(Error: cannot create [%s] group)", entry.path);
		pp_result = PLATFORM_NODE_PARSING_FAILED;
		goto parsing_failed;
	}

	// Update the CGroup variable with kernel info
	cg_result = cgroup_get_cgroup(bbq_node);
	if (cg_result != 0) {
		logger->Error("PLAT LNX: Reading kernel info FAILED! "
				"(Error: %d, %s)", cg_result, cgroup_strerror(cg_result));
		pp_result = PLATFORM_NODE_PARSING_FAILED;
		goto parsing_failed;
	}

	/**********************************************************************
	 *    CPUSET Controller
	 **********************************************************************/

	// Get "cpuset" controller info
	cg_controller = cgroup_get_controller(bbq_node, "cpuset");
	if (cg_controller == NULL) {
		logger->Error("PLAT LNX: Getting controller FAILED! "
				"(Error: Cannot find controller \"cpuset\" "
				"in group [%s])", entry.path);
		pp_result = PLATFORM_NODE_PARSING_FAILED;
		goto parsing_failed;
	}

	// Getting the value for the "cpuset.cpus" attribute
	cg_result = cgroup_get_value_string(cg_controller, BBQUE_LINUXPP_CPUS_PARAM,
			&(prlb->cpus));
	if (cg_result) {
		logger->Error("PLAT LNX: Getting CPUs attribute FAILED! "
				"(Error: 'cpuset.cpus' not configured or not readable)");
		pp_result = PLATFORM_NODE_PARSING_FAILED;
		goto parsing_failed;
	}

	/**********************************************************************
	 *    MEMORY Controller
	 **********************************************************************/

	// Get "memory" controller info
	cg_controller = cgroup_get_controller(bbq_node, "memory");
	if (cg_controller == NULL) {
		logger->Error("PLAT LNX: Getting controller FAILED! "
				"(Error: Cannot find controller \"memory\" "
				"in group [%s])", entry.path);
		pp_result = PLATFORM_NODE_PARSING_FAILED;
		goto parsing_failed;
	}

	// Getting the value for the "memory.limit_in_bytes" attribute
	cg_result = cgroup_get_value_string(cg_controller, BBQUE_LINUXPP_MEMB_PARAM,
			&(prlb->memb));
	if (cg_result) {
		logger->Error("PLAT LNX: Getting MEMORY attribute FAILED! "
				"(Error: 'memory.limit_in_bytes' not configured "
				"or not readable)");
		pp_result = PLATFORM_NODE_PARSING_FAILED;
		goto parsing_failed;
	}

	/**********************************************************************
	 *    CPU Quota Controller
	 **********************************************************************/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)

	// Get "cpu" controller info
	cg_controller = cgroup_get_controller(bbq_node, "cpu");
	if (cg_controller == NULL) {
		logger->Error("PLAT LNX: Getting controller FAILED! "
				"(Error: Cannot find controller \"cpu\" "
				"in group [%s])", entry.path);
		pp_result = PLATFORM_NODE_PARSING_FAILED;
		goto parsing_failed;
	}

	// Getting the value for the "cpu.cfs_quota_us" attribute
	cg_result = cgroup_get_value_string(cg_controller,
			BBQUE_LINUXPP_CPUQ_PARAM, &buff);
	if (cg_result) {
		logger->Error("PLAT LNX: Getting CPU attributes FAILED! "
				"(Error: 'cpu.cfs_quota_us' not configured "
				"or not readable)");
		pp_result = PLATFORM_NODE_PARSING_FAILED;
		goto parsing_failed;
	}

	// Check if a quota has been assigned (otherwise a "-1" is expected)
	if (buff[0] != '-') {

		// Save the "quota" value
		errno = 0;
		prlb->amount_cpuq = strtoul(buff, NULL, 10);
		if (errno != 0) {
			logger->Error("PLAT LNX: Getting CPU attributes FAILED! "
					"(Error: 'cpu.cfs_quota_us' convertion)");
			pp_result = PLATFORM_NODE_PARSING_FAILED;
			goto parsing_failed;
		}

		// Getting the value for the "cpu.cfs_period_us" attribute
		cg_result = cgroup_get_value_string(cg_controller,
				BBQUE_LINUXPP_CPUP_PARAM,
				&buff);
		if (cg_result) {
			logger->Error("PLAT LNX: Getting CPU attributes FAILED! "
					"(Error: 'cpu.cfs_period_us' not configured "
					"or not readable)");
			pp_result = PLATFORM_NODE_PARSING_FAILED;
			goto parsing_failed;
		}

		// Save the "period" value
		errno = 0;
		prlb->amount_cpup = strtoul(buff, NULL, 10);
		if (errno != 0) {
			logger->Error("PLAT LNX: Getting CPU attributes FAILED! "
					"(Error: 'cpu.cfs_period_us' convertion)");
			pp_result = PLATFORM_NODE_PARSING_FAILED;
			goto parsing_failed;
		}

	}

#endif

parsing_failed:
	cgroup_free (&bbq_node);
	return pp_result;
}

LinuxPP::ExitCode_t
LinuxPP::ParseNode(struct cgroup_file_info &entry) {
	RLinuxBindingsPtr_t prlb(new RLinuxBindings_t(0,0));
	ExitCode_t pp_result = OK;

	// Jump all entries deeper than first-level subdirs
	if (entry.depth > 1)
		return OK;

	// Skip parsing of all NON directory, if not required to parse an attribute
	if (entry.type != CGROUP_FILE_TYPE_DIR)
		return OK;

	logger->Info("PLAT LNX: scanning [%d:%s]...",
			entry.depth, entry.full_path);

	// Consistency check for required folder names
	if (strncmp(BBQUE_LINUXPP_CLUSTER, entry.path,
				STRLEN(BBQUE_LINUXPP_CLUSTER))) {
		logger->Warn("PLAT LNX: Resources enumeration, "
				"ignoring unexpected CGroup [%s]",
				entry.full_path);
		return OK;
	}

	pp_result = ParseNodeAttributes(entry, prlb);
	if (pp_result != OK)
		return pp_result;

	// Scan "cpus" and "mems" attributes for each cluster
	logger->Debug("PLAT LNX: Setup resources from [%s]...",
			entry.full_path);

	// Register CPUs for this Node
	pp_result = RegisterCluster(prlb);
	return pp_result;

}

const char*
LinuxPP::_GetPlatformID() {
	static const char linuxPlatformID[] = BBQUE_LINUXPP_PLATFORM_ID;
	return linuxPlatformID;
}

LinuxPP::ExitCode_t
LinuxPP::_LoadPlatformData() {
	struct cgroup *bbq_resources = NULL;
	struct cgroup_file_info entry;
	ExitCode_t pp_result = OK;
	void *node_it = NULL;
	int cg_result;
	int level;

	logger->Info("PLAT LNX: CGROUP based resources enumeration...");

	// Lookup for a "bbque/res" cgroup
	bbq_resources = cgroup_new_cgroup(BBQUE_LINUXPP_RESOURCES);
	cg_result = cgroup_get_cgroup(bbq_resources);
	if (cg_result) {
		logger->Error("PLAT LNX: [" BBQUE_LINUXPP_RESOURCES "] lookup FAILED! "
				"(Error: No resources assignment)");
		return PLATFORM_ENUMERATION_FAILED;
	}

	// Scan  subfolders to map "clusters"
	cg_result = cgroup_walk_tree_begin("cpuset", BBQUE_LINUXPP_RESOURCES,
			1, &node_it, &entry, &level);
	if ((cg_result != 0) || (node_it == NULL)) {
		logger->Error("PLAT LNX: [" BBQUE_LINUXPP_RESOURCES "] lookup FAILED! "
				"(Error: No resources assignment)");
		return PLATFORM_ENUMERATION_FAILED;
	}

	// Scan all "nodeN" assignment
	while (!cg_result && (pp_result == OK)) {
		// That's fine here, since we want also to skip the root group [bbq_resources]
		cg_result = cgroup_walk_tree_next(1, &node_it, &entry, level);
		pp_result = ParseNode(entry);
	}

	// Release the iterator
	cgroup_walk_tree_end(&node_it);

	return pp_result;
}

/*******************************************************************************
 *    Resources Mappign and Assigment to Applications
 ******************************************************************************/

LinuxPP::RLinuxType_t
LinuxPP::GetRLinuxType(br::ResourcePtr_t pres) {
	switch (pres->Name().c_str()[0]) {
	case 'm':
		return RLINUX_TYPE_SMEM;
	case 'p':
		return RLINUX_TYPE_CPU;
	}

	return RLINUX_TYPE_UNKNOWN;
}

uint8_t
LinuxPP::GetRLinuxId(br::ResourcePtr_t pres) {
	const char *pid = pres->Name().c_str();
	unsigned int rid = 0xDEAD;

	// Find ID start
	while (pid && !isdigit(*pid)) {
		++pid;
	}
	// Parse the resource ID
	sscanf(pid, "%u", &rid);
	logger->Debug("Parsing from [%s] => id [%u]",
			pres->Name().c_str(), rid);
	return rid;
}

LinuxPP::ExitCode_t
LinuxPP::ParseBindings(AppPtr_t papp, RViewToken_t rvt,
		RLinuxBindingsPtr_t prlb, br::UsagePtr_t pusage) {
	br::ResourcePtrListIterator_t pres_it;
	char buff[] = "123456789,";
	br::ResourcePtr_t pres;
	unsigned char rid;
	uint64_t usage;

	pres = pusage->GetFirstResource(pres_it);
	while (pres) {

		// Get the resource ID
		rid = GetRLinuxId(pres);
		sprintf(buff, "%d,", rid);

		// Get resource usage amount
		usage = pres->ApplicationUsage(papp, rvt);

		// Set the resource Type
		switch (GetRLinuxType(pres)) {
		case RLINUX_TYPE_SMEM:
			prlb->amount_memb += usage;
			logger->Debug("PLAT LNX: Adding MEMORY %d, "
					"+%llu, total %llu Bytes",
					rid, usage, prlb->amount_memb);
			break;
		case RLINUX_TYPE_CPU:
			prlb->amount_cpus += usage;
			strcat(prlb->cpus, buff);
			logger->Debug("PLAT LNX: Adding CPU %d, "
					"+%llu %, total %llu %",
					rid, usage, prlb->amount_cpus);
			break;
		default:
			// Just to mute compiler warnings..
			break;
		}

		// Get next binded resource
		pres = pusage->GetNextResource(pres_it);
	}

	return OK;

}

LinuxPP::ExitCode_t
LinuxPP::GetResouceMapping(AppPtr_t papp, UsagesMapPtr_t pum,
		RViewToken_t rvt, RLinuxBindingsPtr_t prlb) {
	br::UsagesMap_t::iterator uit;
	br::UsagePtr_t pusage;
	const char *pname;

	// Reset CPUs and MEMORY amounts
	prlb->amount_cpus = 0;
	prlb->amount_memb = 0;

	uit = pum->begin();
	for ( ; uit != pum->end(); ++uit) {
		pname = ((*uit).first).c_str();
		pusage = (*uit).second;

		// Get overall quantity of resource usage
		//rqty = pusage->value;

		// Parse "tile" and "cluster"
		prlb->node_id = br::ResourcePathUtils::GetID(pname, "tile");
		prlb->socket_id = br::ResourcePathUtils::GetID(pname, "cluster");
		logger->Debug("PLAT LNX: Map resources [%s] @ "
				"Node [%d], Socket [%d]",
				pname,
				prlb->node_id, prlb->socket_id);

		// Parse bindings...
		ParseBindings(papp, rvt, prlb, pusage);
	}

	// clean-up leading commas
	prlb->cpus[strlen(prlb->cpus)-1] = 0;
	prlb->mems[strlen(prlb->mems)-1] = 0;

	logger->Debug("PLAT LNX: [%s] => {cpus [%s: %llu %], "
			"mnode[%d: %llu Bytes]}",
			papp->StrId(), prlb->cpus, prlb->amount_cpus,
			prlb->socket_id, prlb->amount_memb);

	return OK;

}


LinuxPP::ExitCode_t
LinuxPP::BuildCGroup(CGroupDataPtr_t &pcgd) {
	int result;

	logger->Debug("PLAT LNX: Building CGroup [%s]...", pcgd->cgpath);

	// Setup CGroup path for this application
	pcgd->pcg = cgroup_new_cgroup(pcgd->cgpath);
	if (!pcgd->pcg) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, \"cgroup\" creation)");
		return MAPPING_FAILED;
	}

	// Add "cpuset" controller
	pcgd->pc_cpuset = cgroup_add_controller(pcgd->pcg, "cpuset");
	if (!pcgd->pc_cpuset) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, [cpuset] \"controller\" "
				"creation failed)");
		return MAPPING_FAILED;
	}
	// Add "memory" controller
	pcgd->pc_memory = cgroup_add_controller(pcgd->pcg, "memory");
	if (!pcgd->pc_memory) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, [memory] \"controller\" "
				"creation failed)");
		return MAPPING_FAILED;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)

	// Add "cpu" controller
	pcgd->pc_cpu = cgroup_add_controller(pcgd->pcg, "cpu");
	if (!pcgd->pc_cpu) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, [cpu] \"controller\" "
				"creation failed)");
		return MAPPING_FAILED;
	}

#endif

	// Create the kernel-space CGroup
	// NOTE: the current libcg API is quite confuse and unclear
	// regarding the "ignore_ownership" second parameter
	logger->Notice("PLAT LNX: Create kernel CGroup [%s]", pcgd->cgpath);
	result = cgroup_create_cgroup(pcgd->pcg, 0);
	if (result && errno) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, kernel cgroup creation "
				"[%d: %s]", errno, strerror(errno));
		return MAPPING_FAILED;
	}

	return OK;

}

LinuxPP::ExitCode_t
LinuxPP::BuildSilosCG(CGroupDataPtr_t &pcgd) {
	RLinuxBindingsPtr_t prlb(new RLinuxBindings_t(
				MaxCpusCount, MaxMemsCount));
	ExitCode_t result;
	int error;

	logger->Debug("PLAT LNX: Building SILOS CGroup...");

	// Build new CGroup data
	pcgd = CGroupDataPtr_t(new CGroupData_t(BBQUE_LINUXPP_SILOS));
	result = BuildCGroup(pcgd);
	if (result != OK)
		return result;

	// Setting up silos (limited) resources, just to run the RTLib
	sprintf(prlb->cpus, "0");
	sprintf(prlb->mems, "0");

	// Configuring silos constraints
	cgroup_set_value_string(pcgd->pc_cpuset,
			BBQUE_LINUXPP_CPUS_PARAM, prlb->cpus);
	cgroup_set_value_string(pcgd->pc_cpuset,
			BBQUE_LINUXPP_MEMN_PARAM, prlb->mems);

	// Updating silos constraints
	logger->Notice("PLAT LNX: Updating kernel CGroup [%s]", pcgd->cgpath);
	error = cgroup_modify_cgroup(pcgd->pcg);
	if (error) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, kernel cgroup update "
				"[%d: %s]", errno, strerror(errno));
		return MAPPING_FAILED;
	}

	return OK;
}

LinuxPP::ExitCode_t
LinuxPP::BuildAppCG(AppPtr_t papp, CGroupDataPtr_t &pcgd) {
	// Build new CGroup data for the specified application
	pcgd = CGroupDataPtr_t(new CGroupData_t(papp));
	return BuildCGroup(pcgd);
}

LinuxPP::ExitCode_t
LinuxPP::GetCGroupData(AppPtr_t papp, CGroupDataPtr_t &pcgd) {
	ExitCode_t result;

	// Loop-up for application control group data
	pcgd = std::static_pointer_cast<CGroupData_t>(
			papp->GetAttribute(PLAT_LNX_ATTRIBUTE, "cgroup")
		);
	if (pcgd)
		return OK;

	// A new CGroupData must be setup for this app
	result = BuildAppCG(papp, pcgd);
	if (result != OK)
		return result;

	// Keep track of this control group
	// FIXME check return value otherwise multiple BuildCGroup could be
	// called for the same application
	papp->SetAttribute(pcgd);

	return OK;

}

LinuxPP::ExitCode_t
LinuxPP::SetupCGroup(CGroupDataPtr_t &pcgd, RLinuxBindingsPtr_t prlb,
		bool excl, bool move) {
	char quota[] = "9223372036854775807";
	char mnode[] = "\09"; // Empty memory node (by default)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
	uint64_t cpus_quota;
#endif
	int result;

	/**********************************************************************
	 *    CPUSET Controller
	 **********************************************************************/

	// Set the assigned CPUs
	cgroup_set_value_string(pcgd->pc_cpuset,
			BBQUE_LINUXPP_CPUS_PARAM, prlb->cpus);
	// Set the assigned memory NODE (only if we have at least one CPUS)
	if (prlb->cpus[0]) {
		snprintf(mnode, 3, "%d", prlb->socket_id);
		cgroup_set_value_string(pcgd->pc_cpuset,
				BBQUE_LINUXPP_MEMN_PARAM, mnode);
	}

#if 0
	// Setting CPUs as EXCLUSIVE if required
	if (excl) {
		cgroup_set_value_string(pcgd->pc_cpuset,
			BBQUE_LINUXPP_CPU_EXCLUSIVE_PARAM, "1");
	}
#else
	excl = false;
#endif

	logger->Debug("PLAT LNX: Setup CPUSET for [%s]: "
			"{cpus [%s: %s], mems[%s]}",
			pcgd->papp->StrId(),
			excl ? 'E' : 'S',
			prlb->cpus, mnode);


	/**********************************************************************
	 *    MEMORY Controller
	 **********************************************************************/

	// Set the assigned MEMORY amount
	sprintf(quota, "%lu", prlb->amount_memb);
	cgroup_set_value_string(pcgd->pc_memory,
			BBQUE_LINUXPP_MEMB_PARAM, quota);

	logger->Debug("PLAT LNX: Setup MEMORY for [%s]: "
			"{bytes_limit [%lu]}",
			pcgd->papp->StrId(), prlb->amount_memb);


	/**********************************************************************
	 *    CPU Quota Controller
	 **********************************************************************/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)

	// Set the default CPU bandwidth period
	cgroup_set_value_string(pcgd->pc_cpu,
			BBQUE_LINUXPP_CPUP_PARAM,
			STR(BBQUE_LINUXPP_CPUP_DEFAULT));

	// Set the assigned CPU bandwidth amount, which is defined by:
	cpus_quota = (BBQUE_LINUXPP_CPUP_DEFAULT / 100) * prlb->amount_cpus;
	cgroup_set_value_uint64(pcgd->pc_cpu,
			BBQUE_LINUXPP_CPUQ_PARAM, cpus_quota);

	logger->Debug("PLAT LNX: Setup CPU for [%s]: "
			"{period [%s], quota [%lu]",
			pcgd->papp->StrId(),
			STR(BBQUE_LINUXPP_CPUP_DEFAULT),
			cpus_quota);

#endif

	/**********************************************************************
	 *    CGroup Configuraiton and Task Assignement
	 **********************************************************************/

	if (move) {

		logger->Notice("PLAT LNX: [%s] => "
				"{cpu [%s: %llu %], mem[%d: %llu B]}",
				pcgd->papp->StrId(),
				prlb->cpus, prlb->amount_cpus,
				prlb->socket_id, prlb->amount_memb);
		cgroup_set_value_uint64(pcgd->pc_cpuset,
				BBQUE_LINUXPP_PROCS_PARAM,
				pcgd->papp->Pid());
	}

	logger->Debug("PLAT LNX: Updating kernel CGroup [%s]", pcgd->cgpath);
	result = cgroup_modify_cgroup(pcgd->pcg);
	if (result) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, kernel cgroup update "
				"[%d: %s])", errno, strerror(errno));
		return MAPPING_FAILED;
	}

	return OK;
}


LinuxPP::ExitCode_t
LinuxPP::_Setup(AppPtr_t papp) {
	RLinuxBindingsPtr_t prlb(new RLinuxBindings_t(
				MaxCpusCount, MaxMemsCount));
	ExitCode_t result = OK;
	CGroupDataPtr_t pcgd;

	// Setup a new CGroup data for this application
	result = GetCGroupData(papp, pcgd);
	if (result != OK) {
		logger->Error("PLAT LNX: [%s] CGroup initialization FAILED "
				"(Error: CGroupData setup)");
		return result;
	}

	// Setup the kernel CGroup with an empty resources assignement
	SetupCGroup(pcgd, prlb, false, false);

	// Reclaim application resource, thus moving this app into the silos
	result = _ReclaimResources(papp);
	if (result != OK) {
		logger->Error("PLAT LNX: [%s] CGroup initialization FAILED "
				"(Error: failed moving app into silos)");
		return result;
	}

	return result;
}

LinuxPP::ExitCode_t
LinuxPP::_Release(AppPtr_t papp) {
	// Release CGroup plugin data
	// ... thus releasing the corresponding control group
	papp->ClearAttribute(PLAT_LNX_ATTRIBUTE);
	return OK;
}

LinuxPP::ExitCode_t
LinuxPP::_ReclaimResources(AppPtr_t papp) {
	RLinuxBindingsPtr_t prlb(new RLinuxBindings_t(MaxCpusCount, MaxMemsCount));
	// FIXME: update once a better SetAttributes support is available
	//CGroupDataPtr_t pcgd(new CGroupData_t);
	CGroupDataPtr_t pcgd;
	int error;

	logger->Debug("PLAT LNX: CGroup resource claiming START");

	// Move this app into "silos" CGroup
	cgroup_set_value_uint64(psilos->pc_cpuset,
			BBQUE_LINUXPP_PROCS_PARAM,
			papp->Pid());

	// Configure the CGroup based on resource bindings
	logger->Notice("PLAT LNX: [%s] => SILOS[%s]",
			papp->StrId(), psilos->cgpath);
	error = cgroup_modify_cgroup(psilos->pcg);
	if (error) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, kernel cgroup update "
				"[%d: %s]", errno, strerror(errno));
		return MAPPING_FAILED;
	}

	logger->Debug("PLAT LNX: CGroup resource claiming DONE!");

	return OK;
}

LinuxPP::ExitCode_t
LinuxPP::_MapResources(AppPtr_t papp, UsagesMapPtr_t pum, RViewToken_t rvt,
		bool excl) {
	RLinuxBindingsPtr_t prlb(new RLinuxBindings_t(MaxCpusCount, MaxMemsCount));
	// FIXME: update once a better SetAttributes support is available
	CGroupDataPtr_t pcgd;
	ExitCode_t result;

	logger->Debug("PLAT LNX: CGroup resource mapping START");

	// Get a reference to the CGroup data
	result = GetCGroupData(papp, pcgd);
	if (result != OK)
		return result;

	result = GetResouceMapping(papp, pum, rvt, prlb);
	if (result != OK) {
		logger->Error("PLAT LNX: binding parsing FAILED");
		return MAPPING_FAILED;
	}
	//prlb->cpus << "7";
	//prlb->mems << "0";

	// Configure the CGroup based on resource bindings
	SetupCGroup(pcgd, prlb, excl, true);

	logger->Debug("PLAT LNX: CGroup resource mapping DONE!");
	return OK;
}

} /* bbque */
