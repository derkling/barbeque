#!/usr/bin/awk -f

BEGIN {
	# Setup Filter Variables
	FS = "|";
	if (!length(BBQUE_UID))  BBQUE_UID="root"
	if (!length(BBQUE_GID))  BBQUE_GID="root"
	if (!length(BBQUE_CPUP)) BBQUE_CPUP="100000"

	# Get IDs of target platform CPUs and MEMs
	if (!length(BBQUE_PLAT_CPUS))  BBQUE_PLAT_CPUS="0"
	if (!length(BBQUE_PLAT_MEMS))  BBQUE_PLAT_MEMS="0"

	# Dump Configuration header
	print "# BBQUE Managed Linux Platform"
	print "# This file has been automatically generated by the BBQUE Platform Builder"

	# Dump the ROOT container configuration
	print  ""
	printf "# BarbequeRTRM Root Container\n"
	printf "group bbque {\n"
	printf "	perm {\n"
	printf "		task {\n"
	printf "			uid = %s;\n", BBQUE_UID
	printf "			gid = %s;\n", BBQUE_GID
	printf "		}\n"
	printf "		admin {\n"
	printf "			uid = %s;\n", BBQUE_UID
	printf "			gid = %s;\n", BBQUE_GID
	printf "		}\n"
	printf "	}\n"
	print  ""
	printf "# This enables configuring a system so that several independent jobs can share\n"
	printf "# common kernel data, such as file system pages, while isolating each job's\n"
	printf "# user allocation in its own cpuset.  To do this, construct a large hardwall\n"
	printf "# cpuset to hold all the jobs, and construct child cpusets for each individual\n"
	printf "# job which are not hardwall cpusets.\n"
	printf "	cpuset {\n"
	printf "		cpuset.cpus = \"%s\";\n", BBQUE_PLAT_CPUS
	printf "		cpuset.mems = \"%s\";\n", BBQUE_PLAT_MEMS
	printf "		cpuset.cpu_exclusive = \"1\";\n"
	printf "		cpuset.mem_exclusive = \"1\";\n"
	printf "	}\n"
	printf "}\n"
	print  ""

} # BBQ Root Container

/^[# -]/ {
	# Jump comments lines
	getline
}

/^HOST/ {

	# Parsing HOST configuration
	gsub(/[[:space:]]*/,"", $2); BBQUE_HOST_CPUS = $2
	gsub(/[[:space:]]*/,"", $3); BBQUE_HOST_MEMS = $3
	 sub(/[[:space:]]*/,"", $4); BBQUE_HOST_DESC = $4

	# Dump the HOST container CGroup
	print  ""
	printf "# BarbequeRTRM HOST CPUs container [%s]\n", BBQUE_HOST_DESC
	printf "group bbque/host {\n"
	printf "	perm {\n"
	printf "		task {\n"
	printf "			uid = %s;\n", BBQUE_UID
	printf "			gid = %s;\n", BBQUE_GID
	printf "		}\n"
	printf "		admin {\n"
	printf "			uid = %s;\n", BBQUE_UID
	printf "			gid = %s;\n", BBQUE_GID
	printf "		}\n"
	printf "	}\n"
	printf "	cpuset {\n"
	printf "		cpuset.cpus = \"%s\";\n", BBQUE_HOST_CPUS
	printf "		cpuset.mems = \"%s\";\n", BBQUE_HOST_MEMS
	printf "	}\n"
	printf "}\n"
	print  ""

} # BBQ Host Device

/^MDEV/ {

	# Parsing MDEV configuration
	gsub(/[[:space:]]*/,"", $2); BBQUE_MDEV_CPUS = $2
	gsub(/[[:space:]]*/,"", $3); BBQUE_MDEV_MEMS = $3
	 sub(/[[:space:]]*/,"", $4); BBQUE_MDEV_DESC = $4

	# Dump the MDEV container CGroup
	print  ""
	printf "# BarbequeRTRM MDEV CPUs Container [%s]\n", BBQUE_MDEV_DESC
	printf "group bbque/res {\n"
	printf "	perm {\n"
	printf "		task {\n"
	printf "			uid = %s;\n", BBQUE_UID
	printf "			gid = %s;\n", BBQUE_GID
	printf "		}\n"
	printf "		admin {\n"
	printf "			uid = %s;\n", BBQUE_UID
	printf "			gid = %s;\n", BBQUE_GID
	printf "		}\n"
	printf "	}\n"
	printf "	cpuset {\n"
	printf "		cpuset.cpus = \"%s\";\n", BBQUE_MDEV_CPUS
	printf "		cpuset.mems = \"%s\";\n", BBQUE_MDEV_MEMS
	printf "	}\n"
	printf "}\n"
	print  ""

} # BBQ Managed CPUs Container

/^NODE/ {

	# Parsing NODE configuration
	gsub(/[[:space:]]*/,"", $2); BBQUE_NODE_CPUS =  $2
	gsub(/[[:space:]]*/,"", $3); BBQUE_NODE_CPUQ = ($3*1000)
	gsub(/[[:space:]]*/,"", $4); BBQUE_NODE_MEMS =  $4
	gsub(/[[:space:]]*/,"", $5); BBQUE_NODE_MEMB = ($5*1048576)

	# Dump the Managed CPUs device configuration
	printf "# Managed CPUs Device, NODE %02d\n", NODE_COUNT
	printf "group bbque/res/node%d {\n", NODE_COUNT
	printf "	perm {\n"
	printf "		task {\n"
	printf "			uid = %s;\n", BBQUE_UID
	printf "			gid = %s;\n", BBQUE_GID
	printf "		}\n"
	printf "		admin {\n"
	printf "			uid = %s;\n", BBQUE_UID
	printf "			gid = %s;\n", BBQUE_GID
	printf "		}\n"
	printf "	}\n"
	printf "	cpuset {\n"
	printf "		cpuset.cpus = \"%s\";\n", BBQUE_NODE_CPUS
	printf "		cpuset.mems = \"%s\";\n", BBQUE_NODE_MEMS
	printf "	}\n"
	if (BBQUE_FEAT_CPUQ != "N") {
		printf "	cpu {\n"
		printf "		cpu.cfs_period_us = \"%s\";\n", BBQUE_CPUP
		printf "		cpu.cfs_quota_us =  \"%s\";\n", BBQUE_NODE_CPUQ
		printf "	}\n"
	}
	printf "	memory {\n"
	printf "		memory.limit_in_bytes = \"%s\";\n", BBQUE_NODE_MEMB
	printf "	}\n"
	printf "}\n"

	NODE_COUNT++;
} # BBQ Managed CPUs Cluster

# vim: set tabstop=4:
