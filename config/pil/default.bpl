# BarbequeRTRM Platform Layout
#
# This is a default platform layout defining resources to be managed by the
# BarbequeRTRM which is suitable for a generic Linux host machine with:
# - 4 CPUs
# - at least 400MB of RAM
#
# The configuration is defined by a set of TARGETs, which are represented by
# lines starting with:
# - HOST: resources assigned to NOT managed tasks
# - MDEV: resources reserved for MANAGED tasks
# - NODE: managed resources clusterization
#
# Columns define RESOURCEs assigned to the corresponding target, which are:
# - CPUs IDs: the numerical ID of assigned CPUs
# - Memory Nodes: the numerical ID of the assigned (NUMA) memory nodes
# - Description: a textual description of the target
# - Time Quota: the allotted PERCENTAGE of CPU time
# - Memory (MB): the alloted Megabytes of MEMORY on the corresponding nodes
#
# Main TARGET devices:
--------+---------------+---------------+---------------------------------------
	| CPUs IDs	| Memory Nodes	| Description
--------+---------------+---------------+---------------------------------------
HOST	| 0		| 0		| BBQUE Generic 1Core Host
MDEV	| 0-3		| 0		| BBQUE Generic 4Core Managed Device
--------+---------------+---------------+---------------------------------------
#
# Resources clusterization for MANAGED resources
--------+---------------+---------------+---------------+-----------------------
	| CPUs IDs	| Time Quota	| Memory Nodes	| Memory (MB)
--------+---------------+---------------+---------------+-----------------------
NODE	| 0-3		| 50		| 0		| 200
NODE	| 0-3		| 50		| 0		| 200
