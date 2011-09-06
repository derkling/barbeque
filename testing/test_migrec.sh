#!/bin/bash

# This should generate a MIGREC case


./BbqRTLibTestApp -r test_migrec_lp -w 15 -c 150 &
sleep 1
./BbqRTLibTestApp -r test_migrec_lp -w 8 -c 80 &
sleep 1
./BbqRTLibTestApp -r test_migrec_lp -w 15 -c 150 &

sleep 5
./BbqRTLibTestApp -r test_migrec_hp -w 10 -c 100

sleep 3
kill -USR1 `ps aux | grep barbeque  | head -n1 | awk '{print $2}'`
kill -USR2 `ps aux | grep barbeque  | head -n1 | awk '{print $2}'`


