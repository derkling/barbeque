#!/bin/bash

./out/usr/bin/BbqRTLibTestApp -r r_02 -w 12 -c 120 &
sleep 1
./out/usr/bin/BbqRTLibTestApp -r r_03 -w 10 -c 100 &
sleep 1
./out/usr/bin/BbqRTLibTestApp -r r_05 -w 10 -c 100 &
sleep 2
./out/usr/bin/BbqRTLibTestApp -r r_01 -w 4 -c 40 &

