#!/bin/bash
BOSP_BASE=${BOSP_BASE:-"/opt/BOSP"}
PARSEC_BASE=${PARSEC_BASE:-"/opt/parsec-2.1"}
FERRET="$PARSEC_BASE/pkgs/apps/ferret/inst/amd64-linux.gcc-pthreads/bin/ferret"

$FERRET \
	$BOSP_BASE/benchmarks/parsec-2.1/pkgs/apps/ferret/inputs/corel \
	lsh \
	$BOSP_BASE/benchmarks/parsec-2.1/pkgs/apps/ferret/inputs/queries \
	10 20 $1 /tmp/ferret.log &>/dev/null

