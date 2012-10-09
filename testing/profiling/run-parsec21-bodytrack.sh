#!/bin/bash
BOSP_BASE=${BOSP_BASE:-"/opt/BOSP"}
PARSEC_BASE=${PARSEC_BASE:-"/opt/parsec-2.1"}
BODYTRACK="$PARSEC_BASE/pkgs/apps/bodytrack/inst/amd64-linux.gcc-pthreads/bin/bodytrack"

$BODYTRACK \
	$BOSP_BASE/benchmarks/parsec-2.1/pkgs/apps/bodytrack/inputs/sequenceB_261 \
	4 40 4000 5 2 $1 &>/dev/null

