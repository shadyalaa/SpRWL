backend=$1 # e.g: herwl
htm_retries=$2 # e.g.: 5
retry_policy=$3
rot_retries=$4 # e.g.: 2 


rm code/tm.h
rm code/thread.c
rm code/thread.h
rm Makefile
rm Makefile.common
rm Makefile.flags
rm Defines.common.mk


if cat /proc/cpuinfo | grep -q "Intel" ;
then
cp ../../backends/common/Defines_Intel.common.mk Defines.common.mk
cp ../../backends/common/Makefile_Intel.flags Makefile.flags
else
cp ../../backends/common/Defines_PPC.common.mk Defines.common.mk
cp ../../backends/common/Makefile_PPC.flags Makefile.flags
fi

cp ../../backends/common/Makefile .
cp ../../backends/common/thread.h code/
cp ../../backends/common/thread.c code/
cp ../../backends/common/tm.h code/
cp ../../backends/common/common_types.h code/
cp common/Makefile.common .
cp ../../backends/$backend/tm.h code/
cp ../../backends/$backend/thread.c code/
cp ../../backends/$backend/thread.h code/
cp ../../backends/$backend/spec.h code/
cp ../../backends/$backend/common_types.h code/

cd code;
rm tpcc

make_command="make HTM_RETRIES=-DHTM_RETRIES=$htm_retries RETRY_POLICY=-DRETRY_POLICY=$retry_policy ROT_RETRIES=-DROT_RETRIES=$rot_retries"

$make_command
