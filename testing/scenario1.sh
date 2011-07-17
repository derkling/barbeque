#!/bin/bash

# This should generate a MIGREC case

./build/Debug/testing/rtlib/rtlib_example01 r_02 12 &
sleep 1
./build/Debug/testing/rtlib/rtlib_example01 r_03 10 &
sleep 1
./build/Debug/testing/rtlib/rtlib_example01 r_05 10 &
sleep 2
./build/Debug/testing/rtlib/rtlib_example01 r_01 4 

