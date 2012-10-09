#!/bin/bash
BOSP_BASE=${BOSP_BASE:-"/opt/BOSP"}
BODYTRACK="$BOSP_BASE/out/usr/bin/parsec/bodytrack"

$BODYTRACK \
	$BOSP_BASE/benchmarks/parsec-2.1/pkgs/apps/bodytrack/inputs/sequenceB_261 \
	4 40 4000 5 2 $1 &>/dev/null

