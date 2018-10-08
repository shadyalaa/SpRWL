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

echo 'ls'

backends[1]="1.0-htm-sgl"
backends[2]="2.0-NoWait"
backends[3]="brlock"
backends[4]="5.2-DynamicWriterWaitN"
backends[5]="5.3-DynamicWriterWaitNS"

benchmarks[1]="reader-u0"
benchmarks[2]="reader-u10"
benchmarks[3]="reader-u50"
benchmarks[4]="reader-u90"
benchmarks[5]="reader-u100"

bStr[1]="hashmap"
bStr[2]="hashmap"
bStr[3]="hashmap"
bStr[4]="hashmap"
bStr[5]="hashmap"

params[1]="-u0 -i800000 -b10000 -r800000 -d1000000 -m1 -n8 -a"
params[2]="-u10 -i800000 -b10000 -r800000 -d1000000 -m1 -n8 -a"
params[3]="-u50 -i800000 -b10000 -r800000 -d1000000 -m1 -n8 -a"
params[4]="-u90 -i800000 -b10000 -r800000 -d1000000 -m1 -n8 -a"
params[5]="-u100 -i800000 -b10000 -r800000 -d1000000 -m1 -n8 -a"

wait_until_finish() {
    pid3=$1
    echo "process is $pid3"
    LIMIT=60
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

#for b is the benchmarks, for c backends
cd $1/benchmarks/datastructures
for c in 1 2 3 4 5
do
	htm_retries="5 10 20"
	retry_policy="0 1 2"
	for h in $htm_retries
	do
		for r in $retry_policy
		do
			bash build-datastructures.sh ${backends[$c]} $h $r
			for b in 1 2 3 4 5 #6 7 8 9 10 11 12 13 14 15
			do
				for t in 1 100 200 300 400 500 600 700 800 900 1000
				do
					for a in {1..5}
					do
		    				echo "${benchmarks[$b]} | ${backends[$c]}-$h-$r | alpha $t | attempt $a"
			    			./hashmap/hashmap ${params[$b]}$t > $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$t-$a.data 2> $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$t-$a.err &
				    		pid3=$!
		    				wait_until_finish $pid3
		    				rc=$?
				    		if [[ $rc != 0 ]] ; then
	    						echo "Error within: | ${benchmarks[$b]} | ${backends[$c]}-$h-$r | alpha $t | attempt $a" >> $runsdir/error.out
				    		fi
					done
				done
			done
		done
	done
done

exit 0
