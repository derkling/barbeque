#!/bin/sh

if [ $# -lt 2 ]; then
	echo  "Usage: $0 <prio> <AWMs>\n\n"
	exit 1
fi

PRIO=$1
AWMs=$2

cat << EOF
<?xml version="1.0"?>
<BarbequeRTRM version="0.4">
  <application priority="$PRIO">
    <awms>
EOF

for i in `seq 1 $AWMs`; do
	V=`printf "%02d" $i` 
	cat <<EOF
        <awm id="$V" name="awm_$V" value="$V">
          <resources>
            <arch name="P2012">
              <tile id="0">
                <cluster id="0">
                  <pe qty="$V"/>
                </cluster>
              </tile>
            </arch>
          </resources>
        </awm>
EOF
done

cat <<EOF
    </awms>
  </application>
</BarbequeRTRM>
EOF

