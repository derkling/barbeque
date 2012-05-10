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
--------+-----------------------+---------------+-------------------------------
	| CPUs IDs		| Memory Nodes	| Description
--------+-----------------------+---------------+-------------------------------
HOST	| 0,4,8,12		| 0		| Not managed Host Device
MDEV	| 1-3,5-7,9-11,13-15	| 1-3		| 3 NUMA NODE Managed Device
--------+---------------+---------------+---------------------------------------
#
# Resources clusterization for MANAGED resources
--------+---------------+---------------+---------------+-----------------------
	| CPUs IDs	| Time Quota	| Memory Nodes	| Memory (MB)
--------+---------------+---------------+---------------+-----------------------
NODE	| 1,5,9,13	| 100		| 3		| 6000
NODE	| 2,6,10,14	| 100		| 2		| 6000
NODE	| 3,7,11,15	| 100		| 1		| 6000

# This configuration has been defined based on the specific machine architecture
#
# CPU type:	AMD Shanghai processor
# *************************************************************
# Hardware Thread Topology
# *************************************************************
# Sockets:	4
# Cores per socket:	4
# Threads per core:	1
# -------------------------------------------------------------
# HWThread	Thread		Core		Socket
# 0		0		0		0
# 1		0		0		3
# 2		0		0		2
# 3		0		0		1
# 4		0		1		0
# 5		0		1		3
# 6		0		1		2
# 7		0		1		1
# 8		0		2		0
# 9		0		2		3
# 10		0		2		2
# 11		0		2		1
# 12		0		3		0
# 13		0		3		3
# 14		0		3		2
# 15		0		3		1
# -------------------------------------------------------------
# Socket 0: ( 0 4 8 12 )
# Socket 1: ( 3 7 11 15 )
# Socket 2: ( 2 6 10 14 )
# Socket 3: ( 1 5 9 13 )
# -------------------------------------------------------------
#
# *************************************************************
# Cache Topology
# *************************************************************
# Level:	1
# Size:	64 kB
# Cache groups:	( 0 ) ( 4 ) ( 8 ) ( 12 ) ( 3 ) ( 7 ) ( 11 ) ( 15 ) ( 2 ) ( 6 ) ( 10 ) ( 14 ) ( 1 ) ( 5 ) ( 9 ) ( 13 )
# -------------------------------------------------------------
# Level:	2
# Size:	512 kB
# Cache groups:	( 0 ) ( 4 ) ( 8 ) ( 12 ) ( 3 ) ( 7 ) ( 11 ) ( 15 ) ( 2 ) ( 6 ) ( 10 ) ( 14 ) ( 1 ) ( 5 ) ( 9 ) ( 13 )
# -------------------------------------------------------------
# Level:	3
# Size:	6 MB
# Cache groups:	( 0 4 8 12 ) ( 3 7 11 15 ) ( 2 6 10 14 ) ( 1 5 9 13 )
# -------------------------------------------------------------
#
# *************************************************************
# NUMA Topology
# *************************************************************
# NUMA domains: 4
# -------------------------------------------------------------
# Domain 0:
# Processors:  0 4 8 12
# Relative distance to nodes:  10 20 20 20
# Memory: 4106.93 MB free of total 8186.19 MB
# -------------------------------------------------------------
# Domain 1:
# Processors:  3 7 11 15
# Relative distance to nodes:  20 10 20 20
# Memory: 6918.36 MB free of total 8192 MB
# -------------------------------------------------------------
# Domain 2:
# Processors:  2 6 10 14
# Relative distance to nodes:  20 20 10 20
# Memory: 6312.95 MB free of total 8192 MB
# -------------------------------------------------------------
# Domain 3:
# Processors:  1 5 9 13
# Relative distance to nodes:  20 20 20 10
# Memory: 3380.86 MB free of total 8192 MB
# -------------------------------------------------------------
