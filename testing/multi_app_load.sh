#!/bin/bash

if [ $# -lt 3 ]; then
	echo -e "\nUsage: $0 <APPS> <EXCs> <TIME>\n\n"
	exit 1
fi

# remove the job failed file
rm -f bbq_test.failed

CYCLES=0
while [ 1 ]; do
	let CYCLES++;

	for a in `seq 1 $1`; do
		./tmp/spawn_apps.sh $2 $3 || exit 2
	done

	# Wait for Jobs to finish
	while [ `jobs -l | wc -l` -ne 0 ]; do
		sleep 2;
	done

	# Check if a job has failed
	if [ -f bbq_test.failed ]; then
		echo "FAILED"
		exit 1
	fi

	echo "\n\nDONE $CYCLES\n\n"
done
