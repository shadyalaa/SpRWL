#!/bin/bash

workspace=$1
cd $1
if [ $4 == "no_random" ];then
resultsdir=$2
runsdir=$resultsdir/runs
else
resultsdir=$2/$RANDOM
mkdir -p $resultsdir/runs
runsdir=$resultsdir/runs
mkdir $resultsdir/plots
mkdir $resultsdir/summary
echo $3 > $resultsdir/desc.txt
fi

if cat /proc/cpuinfo | grep -q "Intel"; then
  thread_counts=( 1 2 4 8 14 28 42 56)
  whouses="-w 56 -m 56"
	items="800000"
else
  thread_counts=( 1 2 4 8 16 32 64 80)
  whouses="-w 80 -m 80"
	items="300000"
fi
attempts=(1 2 3 4 5)


backends[1]="1.0-htm-sgl"       
backends[2]="brlock"            
backends[3]="rwl"               
backends[4]="herwl"
backends[5]="10.1-RHTMO"
backends[6]="10.2-RHTMS"

benchmarks[1]="reader-ll-x1-u0"
benchmarks[2]="reader-ll-x1-u10"
benchmarks[3]="reader-ll-x1-u50"
benchmarks[4]="reader-ll-x1-u90"
benchmarks[5]="reader-ll-x1-u100"

benchmarks[6]="reader-ll-x2-u0"
benchmarks[7]="reader-ll-x2-u10"
benchmarks[8]="reader-ll-x2-u50"
benchmarks[9]="reader-ll-x2-u90"
benchmarks[10]="reader-ll-x2-u100"

benchmarks[11]="reader-ll-x5-u0"
benchmarks[12]="reader-ll-x5-u10"
benchmarks[13]="reader-ll-x5-u50"
benchmarks[14]="reader-ll-x5-u90"
benchmarks[15]="reader-ll-x5-u100"

benchmarks[16]="reader-ll-x10-u0"
benchmarks[17]="reader-ll-x10-u10"
benchmarks[18]="reader-ll-x10-u50"
benchmarks[19]="reader-ll-x10-u90"
benchmarks[20]="reader-ll-x10-u100"

benchmarks[21]="reader-ll-x50-u0"
benchmarks[22]="reader-ll-x50-u10"
benchmarks[23]="reader-ll-x50-u50"
benchmarks[24]="reader-ll-x50-u90"
benchmarks[25]="reader-ll-x50-u100"

benchmarks[26]="reader-ll-x100-u0"
benchmarks[27]="reader-ll-x100-u10"
benchmarks[28]="reader-ll-x100-u50"
benchmarks[29]="reader-ll-x100-u90"
benchmarks[30]="reader-ll-x100-u100"

params[1]="-u0 -i$items -b5000 -r$items -d1000000 -m1 -a"
params[2]="-u10 -i$items -b5000 -r$items -d1000000 -m1 -a"
params[3]="-u50 -i$items -b5000 -r$items -d1000000 -m1 -a"
params[4]="-u90 -i$items -b5000 -r$items -d1000000 -m1 -a"
params[5]="-u100 -i$items -b5000 -r$items -d1000000 -m1 -a"

params[6]="-u0 -i$items -b5000 -r$items -d1000000 -m2 -a"
params[7]="-u10 -i$items -b5000 -r$items -d1000000 -m2 -a"
params[8]="-u50 -i$items -b5000 -r$items -d1000000 -m2 -a"
params[9]="-u90 -i$items -b5000 -r$items -d1000000 -m2 -a"
params[10]="-u100 -i$items -b5000 -r$items -d1000000 -m2 -a"

params[11]="-u0 -i$items -b5000 -r$items -d1000000 -m5 -a"
params[12]="-u10 -i$items -b5000 -r$items -d1000000 -m5 -a"
params[13]="-u50 -i$items -b5000 -r$items -d1000000 -m5 -a"
params[14]="-u90 -i$items -b5000 -r$items -d1000000 -m5 -a"
params[15]="-u100 -i$items -b5000 -r$items -d1000000 -m5 -a"

params[16]="-u0 -i$items -b5000 -r$items -d1000000 -m10 -a"
params[17]="-u10 -i$items -b5000 -r$items -d1000000 -m10 -a"
params[18]="-u50 -i$items -b5000 -r$items -d1000000 -m10 -a"
params[19]="-u90 -i$items -b5000 -r$items -d1000000 -m10 -a"
params[20]="-u100 -i$items -b5000 -r$items -d1000000 -m10 -a"

params[21]="-u0 -i$items -b5000 -r$items -d1000000 -m50 -a"
params[22]="-u10 -i$items -b5000 -r$items -d1000000 -m50 -a"
params[23]="-u50 -i$items -b5000 -r$items -d1000000 -m50 -a"
params[24]="-u90 -i$items -b5000 -r$items -d1000000 -m50 -a"
params[25]="-u100 -i$items -b5000 -r$items -d1000000 -m50 -a"

params[26]="-u0 -i$items -b5000 -r$items -d1000000 -m100 -a"
params[27]="-u10 -i$items -b5000 -r$items -d1000000 -m100 -a"
params[28]="-u50 -i$items -b5000 -r$items -d1000000 -m100 -a"
params[29]="-u90 -i$items -b5000 -r$items -d1000000 -m100 -a"
params[30]="-u100 -i$items -b5000 -r$items -d1000000 -m100 -a"

wait_until_finish() {
				pid3=$1
								echo "process is $pid3"
								LIMIT=300
								for ((j = 0; j < $LIMIT; ++j)); do
												kill -s 0 $pid3
																rc=$u
																if [[ $rc != 0 ]] ; then
																				echo "returning"
																								return;
				fi
								sleep 1s
								done
								kill -9 $pid3
}	



htm_retries=10
retry_policy=0
rot_retry=5

for c in 1 2 3 4 5 6 # backends to runs
do	
	echo "testing "${backends[$c]} >> $resultsdir/desc.txt
	for h in $htm_retries
	do
		for r in $retry_policy
		do
			bash build-datastructures.sh ${backends[$c]} $h $r $rot_retry
			for b in 2 3 4 17 18 19 # benchmarks to run
			do
				for n in "${thread_counts[@]}"
         do
           for a in "${attempts[@]}"
           do
						echo "${benchmarks[$b]} | ${backends[$c]}-$h-$r | threads $n| attempt $a"
						./hashmap/hashmap ${params[$b]} 1 -n $n > $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$n-$a.data 2> $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$n-$a.err &
						pid3=$!
						wait_until_finish $pid3
						wait $pid3
						rc=$?
						if [[ $rc != 0 ]] ; then
							echo "Error within: | ${benchmarks[$b]} | ${backends[$c]}-$h-$r | attempt $a" >> $runsdir/error.out
						fi
					done
				done
			done
		done
	done
done




exit 0
