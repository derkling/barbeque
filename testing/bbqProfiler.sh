#!/bin/bash
#
#       @file  profiling.sh
#      @brief  Barbeque RTRM Profiling
#
# This is a script based Profiling routine which runs the Barbeque RTRM on
# different platform configurations in order to collect performance metrics and
# graph them.
#
#     @author  Patrick Bellasi (derkling), derkling@gmail.com
#
#   @internal
#     Created  09/12/2011
#    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
#     Company  Politecnico di Milano
#   Copyright  Copyright (c) 2011, Patrick Bellasi
#
# This source code is released for free distribution under the terms of the
# GNU General Public License as published by the Free Software Foundation.
# ============================================================================

DATETIME=`date +%Y%m%d_%H%M%S`
DATETIME_STR=`date`
RESULTS=${RESULTS:-"`pwd`/bbqprof_$DATETIME"}
BOSP_BASE=${BOSP_BASE:-"/opt/MyBOSP"}
EXC_RECIPE=${EXC_RECIPE:-"1Awm1PEPrio1"}
EXC_WORKLOAD=${EXC_WORKLOAD:-2}
EXC_CYCLES=${EXC_CYCLES:-20}
COUNT=${COUNT:-30}
BG=${BG:-"true"}

CORES=${CORES:-"1"}
CLUSTERS=${CLUSTERS:-"1 4"}
PES=${PES:-"4 16"}
EXCS=${EXCS:-"4 8 16 32 64"}

# The set of metrics to be collected for graph generation
METRICS="
bq.sp.syncp.avg.time \
bq.sp.syncp.avg.pre  \
bq.sp.syncp.avg.lat  \
bq.sp.syncp.avg.sync \
bq.sp.syncp.avg.do   \
bq.sp.syncp.avg.post \
bq.sm.time           \
bq.sm.yamca.map      \
bq.sm.yamca.entities \
bq.sm.yamca.ord      \
bq.sm.yamca.mcomp    \
bq.sm.yamca.sel      \
"

# Setup other configuration vars
BBQUE_SCRIPT="$RESULTS/bbque_profiling_$USER.sh"
PROGRESS="$RESULTS/bbque_profiling.log"
DRY_RUN=${DRY_RUN:-0}

function bbq_log {
	echo -e "`date`: $1" | tee -a $PROGRESS
}

function dumpConf {
	clear
	echo -e "\t\t=====[ BBQ Profiling Configuration ]=====\n"\
		"\n"\
	        "BOSP Base: $BOSP_BASE\n"\
	        "Recipe: $EXC_RECIPE\n"\
	        "Host Cores: $CORES\n"\
	        "Loops per test: $COUNT\n"\
	        " Clusters: $CLUSTERS\n"\
	        "      PEs: $PES\n"\
	        "     EXCs: $EXCS\n"\
	        "Results dir: $RESULTS\n"\
		"\n" | tee -a $PROGRESS
}

function bbq_stats {
	kill -USR2 `ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep \
		| head -n1 | awk '{print \$2}'`
}

function bbq_running {
	# Look for a BarbequeRTRM running instance
	BBQ_GREP=`ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep \
		| wc -l`
	#bbq_log "Grep: [$BBQ_GREP]"
	[ $BBQ_GREP -eq 0 ] && return 0
	# Report the PID of the BarbequeRTRM instance
	BBQ_PID=`ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep \
		| head -n1 | awk '{print \$2}'`
	bbq_log "  Found BarbequeRTRM instance, PID: $BBQ_PID"
	return 1
}

function bbq_stop {
	sleep 1
	BBQ_PID=`ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep \
		| head -n1 | awk '{print \$2}'`
	bbq_log "  Terminating BBQ (PID: $BBQ_PID)"
	kill -INT $BBQ_PID
}

# Setup the BarbequeRTRM sartup script

# Function parameter:
# CLUSTER; PEs; EXCs; TIME; CYCLES; RECIPE; LOGFILE
function runTest {
	cat > $BBQUE_SCRIPT <<EOF
#!/bin/sh
if [ "x$BG" == "xtrue" ]; then
	stdbuf -oL $BOSP_BASE/out/sbin/barbeque \
		--tpd.clusters $1 --tpd.pes $2 \
		> $RESULTS/$7.log
else
	stdbuf -oL $BOSP_BASE/out/sbin/barbeque \
		--tpd.clusters $1 --tpd.pes $2 \
		| tee $RESULTS/$7.log
fi
EOF
	chmod a+x $BBQUE_SCRIPT

	if [ "x$BG" == "xtrue" ]; then
		$BBQUE_SCRIPT &
	else
		xterm -bg black -fg gray -geometry 143x73+7-34 \
			-title "Barbeque RTMR" $BBQUE_SCRIPT &
	fi
	sleep 1
	for i in `seq 1 $COUNT`; do
		$BOSP_BASE/out/usr/bin/BbqRTLibTestApp -e $3 -w $4 -c $5 -r $6
	done
	while [ 1 ]; do
		bbq_running && break
		bbq_stop
	done
	bzip2 $RESULTS/$7.log
}

function getStats {
	bbq_log "  Distilling stats for $1..."
	bzcat $RESULTS/$1.log.bz2 | $DISTILLER  > $RESULTS/$1.stats
}

# Function parameter:
# Input: CLUSTERS; PEs; EXCs
function testCase {
	LOGFILE=`printf "bbqprof-%s-HC%02d-e%03dw%03dc%03d-C%03dPE%03d" \
		$EXC_RECIPE $CORES $3 $EXC_WORKLOAD $EXC_CYCLES $1 $2`
	bbq_log "Running test $LOGFILE..."
	[ $DRY_RUN -eq 1 ] && return

	runTest $1 $2 $3 $EXC_WORKLOAD $EXC_CYCLES $EXC_RECIPE $LOGFILE
	getStats $LOGFILE
}

# Graph extraction: Metric vs EXCs
# Function parameter: METRIC
function extXexcYmet {
	M=${1:-"bq.sp.syncp.avg.time"}
	cd results
        echo "Extracing data for metric $M..."
	for C in $CLUSTERS; do
	  for P in $PES; do
	    PLAT=`printf "w%03dc%03d-C%03dPE%03d" \
		$EXC_WORKLOAD $EXC_CYCLES $C $P`
	    DATFILE=`printf "graph-%s-%s-HC%02d-%s.dat" \
		    ${M//./_} $EXC_RECIPE $CORES $PLAT`
	    DATHEADER=""
	    DATHEADER="$DATHEADER# BarbequeRTRM (Host Cores: $CORES)\n"
	    DATHEADER="$DATHEADER# Test started at: $DATETIME_STR\n"
	    DATHEADER="$DATHEADER# Metrics: $M\n# Platform: $PLAT\n"
	    DATHEADER="$DATHEADER#EXCs\tMin\tMax\tAvg\tStdDev"
	    echo -e $DATHEADER > $DATFILE
            grep $M *$PLAT.stats \
		    | sed -r 's/-e([0-9]{3}+)w/-e \1 w/' \
		    | tr -s " " \
		    | awk '{printf " %3d %13.3f %13.3f %13.3f %13.3f\n", \
			$2, $12, $14, $16, $18}' \
		>> $DATFILE
	  done
	done
	cd - >/dev/null
}

###############################################################################
#  MAIN
###############################################################################

# Setup results dir
mkdir -p $RESULTS
[ -e results ] && rm -f results
ln -s $RESULTS results

# Dump configuration used
dumpConf

# Setup metrics distiller
DISTILLER="grep -e Metrics -e Description "
for M in $METRICS; do
	DISTILLER="$DISTILLER -e $M"
done

# Run tests
for C in $CLUSTERS; do
  for P in $PES; do
    for E in $EXCS; do
      testCase $C $P $E
    done
  done
done

# Extracing results dataset
for M in $METRICS; do
	extXexcYmet $M
done

rm -f $BBQUE_SCRIPT
bbq_log "\n\nTesting completed\n\n"

