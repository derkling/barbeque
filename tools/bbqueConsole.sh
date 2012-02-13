#!/bin/bash

BBQUE_SYSROOT="/home/derkling/Documents/Coding/MyBOSP"
BBQUE_CONF=bbque.conf_testing
BBQUE_CGLAYOUT=test_root
SUPDAETE=1

# - XWindos geometries recoveder using "xwininfo" utility
# - Font is configured by "-fn" option, to see a list of fonts run:
#     "xlsfonts | less"
FONT=-misc-fixed-medium-r-normal--10-100-75-75-c-60-iso10646-1


cat > $BBQUE_SYSROOT/out/BbqConsole_Resources.sh <<EOF
#!/bin/bash

echo "##### Managed EXCs:"
echo
for APP in \`find $BBQUE_SYSROOT/out/mnt/cgroup/bbque/ -maxdepth 1 -type d\`; do
	NAME=\`basename \$APP\`
	if [ \$NAME == "bbque" -o \$NAME == "silos" -o \$NAME == "res" -o \$NAME == "host" ]; then
		continue
	fi

	CPUS=\`cat \$APP/cpuset.cpus\`
	CPUQ=\`cat \$APP/cpu.cfs_quota_us\`
	CPUP=\`cat \$APP/cpu.cfs_period_us\`
	MEMN=\`cat \$APP/cpuset.mems\`
	MEMB=\`cat \$APP/memory.limit_in_bytes\`
	MEMU=\`cat \$APP/memory.max_usage_in_bytes\`

	let CPUU=(CPUQ * 100)/CPUP

	echo ".:: \$NAME"
	echo "    CPUs: \$CPUS - \${CPUU}% {P: \$CPUP, Q: \$CPUQ}"
	echo "    MEMs: \$MEMB - \${MEMN}n {M: \$MEMU}"
done
EOF
chmod a+x $BBQUE_SYSROOT/out/BbqConsole_Resources.sh

cat > $BBQUE_SYSROOT/out/BbqConsole_CGroups.sh <<EOF
#!/bin/bash

# Main Targest
echo "##### Main TARGET:"
echo
for TRG in \`find $BBQUE_SYSROOT/out/mnt/cgroup/bbque/ -maxdepth 1 -type d\`; do
	NAME=\`basename \$TRG\`
	if [ \$NAME != "bbque" -a \$NAME != "silos" -a \$NAME != "host" -o \$NAME == "res" ]; then
		continue
	fi

	CPUS=\`cat \$TRG/cpuset.cpus\`
	CPUQ=\`cat \$TRG/cpu.cfs_quota_us\`
	CPUP=\`cat \$TRG/cpu.cfs_period_us\`
	MEMN=\`cat \$TRG/cpuset.mems\`
	MEMB=\`cat \$TRG/memory.limit_in_bytes\`
	MEMU=\`cat \$TRG/memory.max_usage_in_bytes\`

	let CPUU=(CPUQ * 100)/CPUP

	echo ".:: \$NAME"
	echo "    CPUs: \$CPUS - \${CPUU}% {P: \$CPUP, Q: \$CPUQ}"
	echo "    MEMs: \$MEMB - \${MEMN}n {Max used: \$MEMU}"

done

# Main Targest
echo
echo "##### Managed NODES:"
echo
for TRG in \`find $BBQUE_SYSROOT/out/mnt/cgroup/bbque/res -maxdepth 1 -type d\`; do
	NAME=\`basename \$TRG\`
	if [ \$NAME == "res" ]; then
		continue
	fi

	CPUS=\`cat \$TRG/cpuset.cpus\`
	CPUQ=\`cat \$TRG/cpu.cfs_quota_us\`
	CPUP=\`cat \$TRG/cpu.cfs_period_us\`
	MEMN=\`cat \$TRG/cpuset.mems\`
	MEMB=\`cat \$TRG/memory.limit_in_bytes\`
	MEMU=\`cat \$TRG/memory.max_usage_in_bytes\`

	let CPUU=(CPUQ * 100)/CPUP

	echo ".:: \$NAME"
	echo "    CPUs: \$CPUS - \${CPUU}[%] {P: \$CPUP, Q: \$CPUQ}"
	echo "    MEMs: \$MEMB - \${MEMN}[nodes] {Max used: \$MEMU}"

done
EOF
chmod a+x $BBQUE_SYSROOT/out/BbqConsole_CGroups.sh

aterm +sb -fn $FONT -geometry 136x18+-9+18 -title "Syslog" \
	-e tail -f /var/log/syslog &
aterm +sb -fn $FONT -geometry 74x39--9-17 -title "EXCs Resources" \
	-e watch -n$SUPDAETE "$BBQUE_SYSROOT/out/BbqConsole_Resources.sh 2>/dev/null "&
aterm +sb -fn $FONT -geometry 74x57--9+18 -title "BBQ CGroups" \
	-e watch -n$SUPDAETE "$BBQUE_SYSROOT/out/BbqConsole_CGroups.sh 2>/dev/null "&
aterm +sb -fn $FONT -geometry 136x77+-9-18 -title "BBQUE Log" \
	-e tail -f $BBQUE_SYSROOT/out/var/bbque/bbque.log &

# BarbequeRTRM daemon startup
aterm +sb -fn $FONT -geometry 105x18+816+18 -title "BarbequeRTRM Daemon" &
cat > $BBQUE_SYSROOT/out/BbqConsole_StartBBQ.sh <<EOF
sudo BBQUE_CGLAYOUT=$BBQUE_CGLAYOUT \
	BBQUE_CONF=$BBQUE_SYSROOT/$BBQUE_CONF \
	./out/etc/init.d/bbqued start
EOF
chmod a+x $BBQUE_SYSROOT/out/BbqConsole_StartBBQ.sh

# First TestApp
aterm +sb -fn $FONT -geometry 106x25+816+240 -title "FIRST TestApp" &
#	-e $BBQUE_SYSROOT/out/usr/bin/BbqRTLibTestApp -w 3 -c 150 -r CPUSonly &
aterm +sb -fn $FONT -geometry 107x25+813+530 -title "SECOND TestApp" &
#	-e $BBQUE_SYSROOT/out/usr/bin/BbqRTLibTestApp -w 3 -c 150 -r CPUSonly &
cat > $BBQUE_SYSROOT/out/BbqConsole_StartTestApp.sh <<EOF
$BBQUE_SYSROOT/out/usr/bin/BbqRTLibTestApp -w 3 -c 150 -r CPUSonly
EOF
chmod a+x $BBQUE_SYSROOT/out/BbqConsole_StartTestApp.sh
