#!/bin/bash
BOSP_BASE=${BOSP_BASE:-"/opt/BOSP"}
FERRET="$BOSP_BASE/out/usr/bin/parsec/ferret"

$FERRET \
	$BOSP_BASE/benchmarks/parsec-2.1/pkgs/apps/ferret/inputs/corel \
	lsh \
	$BOSP_BASE/benchmarks/parsec-2.1/pkgs/apps/ferret/inputs/queries \
	10 20 $1 /tmp/ferret.log &>/dev/null

