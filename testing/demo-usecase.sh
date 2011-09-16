#!/bin/bash

BOSP_BASE=${BOSP_BASE:-"/opt/MyBOSP"}

BBQ_TESTAPP=$BOSP_BASE/out/usr/bin/BbqRTLibTestApp

################################################################################
###  Demo Logging Support
################################################################################

# Logging
PROGRESS="./usecase.log"
function DemoLog {
	echo -e "`date`: $1" | tee -a $PROGRESS
}


################################################################################
###  Demo Setup and Cleanup
################################################################################

# Setup TestApp name for better logtrace deadability
function Setup {

	for i in `seq 1 20`; do
		APPNAME=`printf "TestApp_%03d.sh" $i`
		ln -s $BBQ_TESTAPP $APPNAME
	done
}

function Cleanup {
	rm -rf TestApp_*.sh
}


################################################################################
###  Demo Simulation Time
################################################################################

SIMTIME=0
# Simulatioon time advancement by the specified amount of [s]
# SEC;
function AdvanceSimTime {
	sleep $1

	let SIMTIME=SIMTIME+$1
	TIMELOG=`printf "=====[ Simulation Time: %3d[s] ]=====" $SIMTIME`

	DemoLog ""
	DemoLog "$TIMELOG"
}


################################################################################
###  BarbequeRTRM Management stuff
################################################################################

# Host platform configuration (just for logging)
CORES=${CORES:-1}

# Target platform configuration (Test Platform Data)
CLUSTERS=${CLUSTERS:-4}
PES=${PES:-10}

# Set true this var to run Barbeque in Background
BG=${BG:-"false"}

BBQUE_SCRIPT="`pwd`/bbque_$USER.sh"
BBQUE_LOGFILE=`printf "demo-usecase-HC%02d-C%03dPE%03d" $CORES $CLUSTERS $PES`

function BbqStats {
	kill -USR2 `ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep \
		| head -n1 | awk '{print \$2}'`
}

function BbqRunning {
	# Look for a BarbequeRTRM running instance
	BBQ_GREP=`ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep \
		| wc -l`
	#bbq_log "Grep: [$BBQ_GREP]"
	[ $BBQ_GREP -eq 0 ] && return 0
	# Report the PID of the BarbequeRTRM instance
	BBQ_PID=`ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep \
		| head -n1 | awk '{print \$2}'`
	DemoLog "  Found BarbequeRTRM instance, PID: $BBQ_PID"
	return 1
}

function BbqStop {
	sleep 1
	BBQ_PID=`ps aux | grep "sbin/barbeque " | grep $USER | grep -v grep \
		| head -n1 | awk '{print \$2}'`
	DemoLog "  Terminating BBQ (PID: $BBQ_PID)"
	kill -INT $BBQ_PID
}

function BbqTermiante {
	while [ 1 ]; do
		BbqRunning && break
		BbqStop
	done
	bzip2 $BBQUE_LOGFILE.log
}

# Setup BBQ startup script
cat > $BBQUE_SCRIPT <<EOF
#!/bin/sh
if [ "x$BG" == "xtrue" ]; then
	stdbuf -oL $BOSP_BASE/out/sbin/barbeque \
		--tpd.clusters $CLUSTERS --tpd.pes $PES \
		> $BBQUE_LOGFILE.log
else
	stdbuf -oL $BOSP_BASE/out/sbin/barbeque \
		--tpd.clusters $CLUSTERS --tpd.pes $PES \
		| tee $BBQUE_LOGFILE.log
fi
EOF
chmod a+x $BBQUE_SCRIPT

# Function parameter:
function BbqStart {

	DemoLog "Using Barbeque Logfile: $BBQUE_LOGFILE"
	if [ "x$BG" == "xtrue" ]; then
		$BBQUE_SCRIPT &
	else
		xterm -bg black -fg gray -geometry 143x73+7-34 \
			-title "Barbeque RTMR" $BBQUE_SCRIPT &
	fi
	sleep 3
}

################################################################################
###  Test Application Functions
################################################################################

HP=0
LP=1
ENDLESS=30

# Input parameters
# APP; PRIO: {0,1}; EXCS [count]; DURATION [s];
function SpawnApp {
	APPNAME=`printf "./TestApp_%03d" $1`
	let CYCLES=10*$4

	DemoLog "Starting application [$1]..."
	if [ $2 -eq $HP ]; then
		$APPNAME.sh -r rUseCase4_HP -e $3 -w $4 -c $CYCLES  > $APPNAME.log &
	else
		$APPNAME.sh -r rUseCase4_LP -e $3 -w $4 -c $CYCLES  > $APPNAME.log &
	fi
}


################################################################################
###  Test MAIN
################################################################################

DemoLog ""
DemoLog "=====[ Demo scenarios SETUP"
Setup

DemoLog ""
DemoLog "=====[ Starting BarbequeRTRM..."
BbqStart

AdvanceSimTime 1
SpawnApp 1 $LP 1 1
SpawnApp 2 $LP 1 2
SpawnApp 3 $HP 1 $ENDLESS
SpawnApp 4 $LP 1 2

AdvanceSimTime 1
SpawnApp 5 $LP 1 5
SpawnApp 6 $HP 1 5

AdvanceSimTime 1
SpawnApp 7 $LP 1 2

AdvanceSimTime 1
SpawnApp 8 $LP 1 1
SpawnApp 9 $LP 1 1

AdvanceSimTime 1
SpawnApp 10 $LP 1 5

AdvanceSimTime 1
SpawnApp 11 $HP 1 2

AdvanceSimTime 1
SpawnApp 12 $LP 1 $ENDLESS

AdvanceSimTime 1
SpawnApp 13 $LP 1 2
SpawnApp 14 $HP 1 2
SpawnApp 15 $LP 1 2

AdvanceSimTime 1
SpawnApp 16 $LP 1 4
SpawnApp 17 $LP 1 2

AdvanceSimTime 1
SpawnApp 18 $LP 1 1
SpawnApp 19 $HP 1 1

AdvanceSimTime 1
SpawnApp 20 $LP 1 5

# Wait for all apps to finish
AdvanceSimTime $ENDLESS

#cleanup everyting
BbqTermiante
Cleanup

