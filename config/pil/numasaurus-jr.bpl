# BarbequeRTRM Platform Layout
#
# This is a default platform layout defining resources to be managed by the
# BarbequeRTRM which is suitable for a generic Linux NUMA host machine with:
# - 4 NUMA nodes, with 4 CPUs each one
# - 6GB of RAM for each NODE
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
HOST	| 0-3		| 0		| BBQUE Managed NSJ Host
MDEV	| 4-15		| 1-3		| BBQUE 3 NUMA NODE Managed Device
--------+---------------+---------------+---------------------------------------
#
# Resources clusterization for MANAGED resources
--------+---------------+---------------+---------------+-----------------------
	| CPUs IDs	| Time Quota	| Memory Nodes	| Memory (MB)
--------+---------------+---------------+---------------+-----------------------
NODE	| 4-7		| 100		| 1		| 6000
NODE	| 8-11		| 100		| 2		| 6000
NODE	| 12-15		| 100		| 3		| 6000
