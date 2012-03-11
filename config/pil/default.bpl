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
HOST	| 0-1		| 0		| BBQUE Generic 2Core Host
MDEV	| 2-3		| 0		| BBQUE Generic 2Core Managed Device
--------+---------------+---------------+---------------------------------------
#
# Resources clusterization for MANAGED resources
--------+---------------+---------------+---------------+-----------------------
	| CPUs IDs	| Time Quota	| Memory Nodes	| Memory (MB)
--------+---------------+---------------+---------------+-----------------------
NODE	| 2-3		| 100		| 0		| 800
