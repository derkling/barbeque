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

#include "bbque/res/resource_utils.h"

#include <string.h>

namespace bb = bbque;
namespace br = bbque::res;

namespace bbque {

LinuxPP::LinuxPP() :
	PlatformProxy(),
	controller("cpuset"),
	MaxCpusCount(DEFAULT_MAX_CPUS),
	MaxMemsCount(DEFAULT_MAX_MEMS) {
	ExitCode_t pp_result;
	char *mount_path;
	int cg_result;

	// Init the Control Group Library
	cg_result = cgroup_init();
	if (cg_result) {
		logger->Error("PLAT LNX: CGroup Library initializaton FAILED! "
				"(Error: %d - %d)", cg_result, cgroup_strerror(cg_result));
		return;
	}

	cgroup_get_subsys_mount_point(controller, &mount_path);
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
}

LinuxPP::~LinuxPP() {

}

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
LinuxPP::GetResouceMapping(AppPtr_t papp, UsagesMapPtr_t pum,
		RViewToken_t rvt, RLinuxBindingsPtr_t prlb) {
	br::ResourcePtrList_t::iterator rit;
	br::UsagesMap_t::iterator uit;
	br::ResourcePtr_t pres;
	br::UsagePtr_t pusage;
	const char *pname;
	unsigned char rid;
	char buff[] = "123456789,";

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
		rit = pusage->binds.begin();
		for ( ; rit != pusage->binds.end(); ++rit) {
			pres = (*rit);

			// Jump resources not assigned to this application
			if (!pres->ApplicationUsage(papp, rvt))
				continue;

			// Get the resource ID
			rid = GetRLinuxId(pres);
			sprintf(buff, "%d,", rid);

			// Set the resource Type
			switch (GetRLinuxType(pres)) {
			case RLINUX_TYPE_SMEM:
				strcat(prlb->mems, buff);
				logger->Debug("PLAT LNX: adding MEMORY %d", rid);
				break;
			case RLINUX_TYPE_CPU:
				strcat(prlb->cpus, buff);
				logger->Debug("PLAT LNX: adding CPU %d", rid);
				break;
			default:
				// Just to mute compiler warnings..
				break;
			}
		}
	}

	// clean-up leading commas
	prlb->cpus[strlen(prlb->cpus)-1] = 0;
	prlb->mems[strlen(prlb->mems)-1] = 0;

	logger->Notice("PLAT LNX: [%s] => {cpus [%s], mems [%s]}",
			papp->StrId(), prlb->cpus, prlb->mems);

	return OK;

}


LinuxPP::ExitCode_t
LinuxPP::BuildCGroup(CGroupDataPtr_t &pcgd) {
	int result;

	// Setup CGroup path for this application
	pcgd->pcg = cgroup_new_cgroup(pcgd->cgpath);
	if (!pcgd->pcg) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, \"cgroup\" creation)");
		return MAPPING_FAILED;
	}

	// Add "cpuset" controller
	pcgd->pc_cpuset = cgroup_add_controller(pcgd->pcg, "cpuset");
	if (!pcgd->pcg) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, [cpuset] \"controller\" creation)");
		return MAPPING_FAILED;
	}

	// Create the kernel-sapce CGroup
	logger->Notice("PLAT LNX: Create kernel CGroup [%s]", pcgd->cgpath);
	result = cgroup_create_cgroup(pcgd->pcg, 0);
	if (result) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, kernel cgroup creation "
				"[%d: %s]", errno, strerror(errno));
		return MAPPING_FAILED;
	}

	return OK;

}

LinuxPP::ExitCode_t
LinuxPP::BuildSilosCG(CGroupDataPtr_t &pcgd) {
	RLinuxBindingsPtr_t prlb(new RLinuxBindings_t(MaxCpusCount, MaxMemsCount));
	ExitCode_t result;
	int error;

	// Build new CGroup data
	pcgd = CGroupDataPtr_t(new CGroupData_t("bbq_silos"));
	result = BuildCGroup(pcgd);
	if (result != OK)
		return result;

	// Setting up silos (limited) resources, just to run the RTLib
	sprintf(prlb->cpus, "0");
	sprintf(prlb->mems, "0");

	// Configuring silos constraints
	cgroup_set_value_string(pcgd->pc_cpuset, "cpus", prlb->cpus);
	cgroup_set_value_string(pcgd->pc_cpuset, "mems", prlb->mems);

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
	// FIXME check return value otherwise multiple BuildCGroup could be called
	// for the same application
	papp->SetAttribute(pcgd);

	return OK;

}

LinuxPP::ExitCode_t
LinuxPP::SetupCGroup(CGroupDataPtr_t &pcgd, RLinuxBindingsPtr_t prlb,
		bool move) {
	int result;

	cgroup_set_value_string(pcgd->pc_cpuset, "cpus", prlb->cpus);
	cgroup_set_value_string(pcgd->pc_cpuset, "mems", prlb->mems);

	if (move) {
		logger->Notice("PLAT LNX: [%s] => {cpus [%s], mems [%s]}",
				pcgd->papp->StrId(), prlb->cpus, prlb->mems);
		cgroup_set_value_uint64(pcgd->pc_cpuset, "cgroup.procs",
				pcgd->papp->Pid());
	}

	logger->Debug("PLAT LNX: Updating kernel CGroup [%s]", pcgd->cgpath);
	result = cgroup_modify_cgroup(pcgd->pcg);
	if (result) {
		logger->Error("PLAT LNX: CGroup resource mapping FAILED "
				"(Error: libcgroup, kernel cgroup update "
				"[%d: %s]", errno, strerror(errno));
		return MAPPING_FAILED;
	}

	return OK;
}


LinuxPP::ExitCode_t
LinuxPP::_Setup(AppPtr_t papp) {
	RLinuxBindingsPtr_t prlb(new RLinuxBindings_t(MaxCpusCount, MaxMemsCount));
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
	SetupCGroup(pcgd, prlb, false);

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
	cgroup_set_value_uint64(psilos->pc_cpuset, "cgroup.procs", papp->Pid());

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
	SetupCGroup(pcgd, prlb);

	logger->Debug("PLAT LNX: CGroup resource mapping DONE!");
	return OK;
}

} /* bbque */
