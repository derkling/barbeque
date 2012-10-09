#!/bin/bash
#
# Copyright (C) 2012  Politecnico di Milano
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>

# Test Configuration settings
APP=${APP:-"bodytrack ferret"}
NUMS=${NUMS:-"1"}
NTHREADS=${NTHREADS:-"12"}
BBQ=${BBQ:-0}
COUNT=${COUNT:-1}

# Other internal settings
BOSP_BASE=${BOSP_BASE:-"/opt/BOSP"}
DATETIME=`date +%Y%m%d_%H%M%S`
DATETIME_STR=`date`
BBQ_STR=${BBQ:-"NOBBQ"}
CPUFREQGOV=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
CPUFREQCUR=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq`


function print_header {
	echo -e "#=========================================" | tee -a $OUTFILE
	echo -e "# PARSEC Workload        : $3" | tee -a $OUTFILE
	echo -e "# Number of instances    : $1" | tee -a $OUTFILE
	echo -e "# Number of threads (max): $2" | tee -a $OUTFILE
	echo -e "# Number of Tests        : $COUNT" | tee -a $OUTFILE
	echo -e "# CPUfreq governor       : $CPUFREQGOV" | tee -a $OUTFILE
	echo -e "# CPUfreq frequency (Hz) : $CPUFREQCUR" | tee -a $OUTFILE
	echo -e "# BarbequeRTRM running   : $YN" | tee -a $OUTFILE
	echo -e "# Test date: $DATETIME_STR" | tee -a $OUTFILE
	echo -e "#-----------------------------------------" | tee -a $OUTFILE
	echo -e "# Test#     Time[s]  Pow[W] Perf_Stats... " | tee -a $OUTFILE
	echo -e "#-----------------------------------------" | tee -a $OUTFILE
}

BSCRIPT="/tmp/bbque_perftest.sh"
# Arguments:
# $1 = number of instances to launch
# $2 = the benchmark command to run
# $3 = max number of threads
cat > $BSCRIPT <<EOF
#!/bin/sh
for n in \`seq 1 \$1\`; do
	eval "\$2 \$3 &"
done
echo "Wait for all the testapp to finish..."
wait
EOF
chmod a+x $BSCRIPT

PSCRIPT="/tmp/bbque_perfparse.awk"
cat > $PSCRIPT <<EOF
#!/usr/bin/awk -f
/task-clock/{
	printf \$1" "\$4" "; getline;
	printf \$1" "; getline;
	printf \$1" "; getline;
	printf \$1" "; getline;
	printf \$1" "\$4" "; getline;
	printf \$1" "\$4" "; getline;
	printf \$1" "\$4" "; getline;
	printf \$1" "\$4" "; getline;
		printf \$2" "; getline;
	printf \$1" "\$4" "; getline;
	printf \$1" "\$4" "; getline;
	getline;
	printf "; "\$1;
}
EOF
chmod a+x $PSCRIPT


# Arguments:
# $1 = number of instances to launch
# $2 = max number of threads
# $3 = number of test cycle (repetition)
# $4 = the workload application
function run_test {
	# Dump Test count
	printf "%7d\t" $3 | tee -a $OUTFILE

	# Run Test
	perf stat -o /tmp/perf.log $BSCRIPT $1 $CMDWORK $2 >/dev/null

	# Get Current Power consumption
	POWER=`ipmitool sdr list full | grep "System Level" | awk '{print \$4}'`

	# Get Test completion time from PERF data
	PERF_DATA=`cat /tmp/perf.log | $PSCRIPT | tr -d ,%`
	TIME=`echo $PERF_DATA | awk -F";" '{print $2}'`

	# Dump test results on logfile
	printf "%12.6f %4d   " $TIME $POWER | tee -a $OUTFILE
	echo $PERF_DATA | awk -F";" '{print $1}' | tee -a $OUTFILE

	# Wait few seconds to allows the system to cool down
	sleep 1
}

function setup_env {

# Make sure only root can run this test
if [ "$(id -u)" != "0" ]; then
	echo "This profiling test requires ROOT permissions!" 1>&2
	echo "Please, restart it as root."
	echo
	exit 1
fi

# Ensure the task is assigned to the Managed Resources partition
# in case we are running without BBQ control
if [ $BBQ -eq 0 ]; then
	echo $$ > ../../../out/mnt/cgroup/bbque/res/tasks
fi

# Source the BOSP shell configuration
source $BOSP_BASE/out/etc/bbque/bosp_init.env

}

###############################################################################
#  MAIN
###############################################################################

# Setup execution environment
setup_env

# Output directory
OUTDIR="ParsecTest-$DATETIME"
mkdir $OUTDIR

# For all the workloads...
for a in $APP; do
	# Run the 'vanilla' or the BBQ-integrated version of the workload
	if [ $BBQ -eq 1 ];
	then
		YN="Yes"
		BBQ_STR="BBQ"
		CMDWORK="./run-parsec21-${a}_bbq.sh"
	else
		YN="No"
		BBQ_STR="NOBBQ"
		CMDWORK="./run-parsec21-${a}.sh"
	fi

	# Number of instances to launch in parallel
	for n in $NUMS; do
		# Iterate for the number of threads
		for t in $NTHREADS; do
			OUTFILE=`printf "$OUTDIR/PARSECTest-$a-N%02d-T%02d-$BBQ_STR.dat" $n $t`

			# Print the header into the output file and start
			print_header $n $t $a
			for c in `seq 1 $COUNT`; do
				run_test $n $t $c $a
			done
		done
		echo "Waiting 3[s] before continue..."
		sleep 3
	done
done
