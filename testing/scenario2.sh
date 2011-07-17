#!/bin/bash

# This should generate a MIGREC case

./build/Debug/testing/rtlib/rtlib_example01 r_02 12 &
./build/Debug/testing/rtlib/rtlib_example01 r_03 10 &
sleep 1
./build/Debug/testing/rtlib/rtlib_example01 r_00 10 &
sleep 3
./build/Debug/testing/rtlib/rtlib_example01 r_01 4 

