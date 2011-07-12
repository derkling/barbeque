#!/bin/bash

if [ $# -lt 2 ]; then
	echo -e "\nUsage: $0 <EXCs_COUNT> <EXC_Time>\n\n"
	exit 1
fi

./testing/rtlib/rtlib_testapp  $1 1 1 1 $2 || touch bbq_test.failed &

