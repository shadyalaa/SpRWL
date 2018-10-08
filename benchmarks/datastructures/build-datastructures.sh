#!/bin/sh
#FOLDERS="skiplist linkedlist redblacktree hashmap"
FOLDERS="hashmap linkedlist"
if [ $# -eq 0 ] ; then
    echo " === ERROR At the very least, we need the backend name in the first parameter. === "
    exit 1
fi

backend=$1  # e.g.: greentm

rot_retries=5
htm_retries=10
htm_capacity_abort_strategy=2

if [ $# -eq 4 ] ; then
    htm_retries=$2 # e.g.: 5
    htm_capacity_abort_strategy=$3 # e.g.: 0 for "give up"
		rot_retries=$4
fi

if [ $# -eq 3 ] ; then
    htm_retries=$2 # e.g.: 5
    htm_capacity_abort_strategy=$3 # e.g.: 0 for "give up"
fi

rm lib/*.o || true

rm Defines.common.mk
rm Makefile
rm Makefile.flags
rm lib/thread.h
rm lib/thread.c
rm lib/tm.h

if cat /proc/cpuinfo | grep -q "Intel" ;
then
	cp ../../backends/common/Defines_Intel.common.mk Defines.common.mk
	cp ../../backends/common/Makefile_Intel.flags Makefile.flags
else
	cp ../../backends/common/Defines_PPC.common.mk Defines.common.mk
	cp ../../backends/common/Makefile_PPC.flags Makefile.flags
fi

cp ../../backends/common/Makefile .
cp ../../backends/common/thread.h lib/
cp ../../backends/common/thread.c lib/
cp ../../backends/common/tm.h lib/
cp ../../backends/common/common_types.h lib/

cp ../../backends/$backend/thread.h lib/
cp ../../backends/$backend/thread.c lib/
cp ../../backends/$backend/tm.h lib/
cp ../../backends/$backend/spec.h lib/


for F in $FOLDERS
do
    cd $F
    rm *.o || true
    rm $F
    make_command="make -f Makefile TSTATS=-DTIME_STATS HTM_RETRIES=-DHTM_RETRIES=$htm_retries RETRY_POLICY=-DRETRY_POLICY=$htm_capacity_abort_strategy ROT_RETRIES=-DROT_RETRIES=$rot_retries"
    $make_command
    rc=$?
    if [[ $rc != 0 ]] ; then
        echo ""
        echo "=================================== ERROR BUILDING $F - $name ===================================="
        echo ""
        exit 1
    fi
    cd ..
done

