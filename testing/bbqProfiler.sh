#!/bin/bash
#
#       @file  profiling.sh
#      @brief  Barbeque RTRM Profiling
#
# This is a script based Profiling routine which runs the Barbeque RTRM on
# different platform configurations in order to collect performance metrics and
# graph them.
#
#     @author  Patrick Bellasi (derkling), derkling@google.com
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

RESULTS=${RESULTS:-"`pwd`/bbqprof_`date +%Y%m%d_%k%M%S`"}
BOSP_BASE=${BOSP_BASE:-"/opt/MyBOSP"}
EXC_RECIPE=${EXC_RECIPE:-"1Awm1PEPrio1"}
EXC_WORKLOAD=${EXC_WORKLOAD:-2}
EXC_CYCLES=${EXC_CYCLES:-20}
COUNT=${COUNT:-30}

CLUSTERS=${CLUSTERS:-"1 2 4 8 16"}
PES=${PES:-"4 8 16 32"}
EXCS=${EXCS:-"10 20 30 40 50 60 70 80 90"}

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
PROGRESS="$RESULTS/pprogress.log"
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
	        "Loops per test: $COUNT\n"\
	        " Clusters: $CLUSTERS\n"\
	        "      PEs: $PES\n"\
	        "     EXCs: $EXCS\n"\
	        "Results dir: $RESULTS\n"\
		"\n" | tee -a $PROGRESS
}

function bbq_stats {
	kill -USR2 `ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep | head -n1 | awk '{print \$2}'`
}

function bbq_running {
	BBQ_GREP=`ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep | wc -l`
	#bbq_log "Grep: [$BBQ_GREP]"
	BBQ_PID=`ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep | head -n1 | awk '{print \$2}'`
	bbq_log "Found BarbequeRTRM instance, PID: $BBQ_PID"
	[ $BBQ_GREP -gt 0 ] && return 1
	return 0
}

function bbq_stop {
	sleep 1
	BBQ_PID=`ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep | head -n1 | awk '{print \$2}'`
	bbq_log "Terminating BBQ (PID: $BBQ_PID)"
	kill -INT $BBQ_PID
}

# Function parameter:
# CLUSTER; PEs; EXCs; TIME; CYCLES; RECIPE; LOGFILE
function runTest {
	cat > $BBQUE_SCRIPT <<EOF
#!/bin/sh
#stdbuf -oL $BOSP_BASE/out/sbin/barbeque --tpd.clusters $1 --tpd.pes $2 > $7.log
stdbuf -oL $BOSP_BASE/out/sbin/barbeque --tpd.clusters $1 --tpd.pes $2 | tee $RESULTS/$7.log
EOF
	chmod a+x $BBQUE_SCRIPT

	xterm -bg black -fg gray -geometry 143x73+7-34 -title "Barbeque RTMR" $BBQUE_SCRIPT &
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
	LOGFILE=`printf "bbqprof_%s-e%03dw%03dc%03d-C%03dPE%03d" \
		$EXC_RECIPE $3 $EXC_WORKLOAD $EXC_CYCLES $1 $2`
	bbq_log "Running test $LOGFILE..."
	[ $DRY_RUN -eq 1 ] && return

	runTest $1 $2 $3 $EXC_WORKLOAD $EXC_CYCLES $EXC_RECIPE $LOGFILE
	getStats $LOGFILE
}

# Graph extraction: Metric vs EXCs
# Function parameter: METRIC
function extXexcYmet {
	M=${1:-"bq.sp.syncp.avg.time"}
	G=${M//./_}
	cd results
        echo "Extracing data for metric $M..."
	for C in $CLUSTERS; do
	  for P in $PES; do
	    PLAT=`printf "w%03dc%03d-C%03dPE%03d" \
		$EXC_WORKLOAD $EXC_CYCLES $C $P`
	    echo -e \
	         "# Metrics: $M\n# Platform: $PLAT\n#EXCs          Min           Max           Avg        StdDev"\
		 > graph_$G-$PLAT.dat
            grep $M *$PLAT.stats | sed -r 's/-e([0-9]{3}+)w/-e \1 w/' | tr -s " " | \
                awk '{printf " %3d %13.3f %13.3f %13.3f %13.3f\n", $2, $12, $14, $16, $18}' \
		>> graph_$G-$PLAT.dat
	  done
	done
	cd - >/dev/null
}

###############################################################################
#  MAIN
###############################################################################

# Setup results dir
mkdir -p $RESULTS
ln -s $RESULTS results

# Dump configuration used
dumpConf

# Setup metrics distiller
DISTILLER="grep -e Metrics -e Description "
for M in $METRICS; do
	DISTILLER="$DISTILLER -e $M"
done

# Extracing results dataset
for M in $METRICS; do
	extXexcYmet $M
done
exit 0

# Run tests
for C in $CLUSTERS; do
  for P in $PES; do
    for E in $EXCS; do
      testCase $C $P $E
    done
  done
done

bbq_log "\n\nTesting completed\n\n"

