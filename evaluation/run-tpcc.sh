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

backends[1]="1.0-htm-sgl"       
backends[2]="brlock"            
backends[3]="rwl"               
backends[4]="herwl"
backends[5]="10.1-RHTMO"
backends[6]="10.2-RHTMS"

benchmarks[1]="tpcc-stn"

params[1]="-s 31 -d 4 -o 4 -p 43 -r 18"


wait_until_finish() {
    pid3=$1
    echo "process is $pid3"
    LIMIT=300
    for ((j = 0; j < $LIMIT; ++j)); do
        kill -s 0 $pid3
        rc=$u
        if [[ $rc != 0 ]] ; then
            echo "returning"
            rbenchmarks[17]="tpcc-hh-sp-u10"eturn;
        fi
        sleep 1s
    done
    kill -9 $pid3
}	

htm_retries=10
retry_policy=0
rot_retry=5
if cat /proc/cpuinfo | grep -q "Intel"; then
	thread_counts=( 1 2 4 8 14 28 42 56)
	whouses="-w 56 -m 56"
else
	thread_counts=( 1 2 4 8 16 32 64 80)
	whouses="-w 80 -m 80"
fi
attempts=(1 2 3 4 5)


cd $1/benchmarks/tpcc
for c in 1 2 3 4 5 6 # backends to runs
do
	echo "testing "${backends[$c]}" with alphas: ""${alphas[@]}">> $resultsdir/desc.txt
	for h in $htm_retries
	do
		for r in $retry_policy
		do
			bash build-tpcc.sh ${backends[$c]} $h $r $rot_retry
			for b in 1
			do
				for n in "${thread_counts[@]}"
				do
					for a in "${attempts[@]}"
						do
						echo "${benchmarks[$b]} | ${backends[$c]}-$h-$r | threads $n | attempt $a"
			    	./code/tpcc -t 5 $whouses ${params[$b]} -n $n > $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$n-$a.data 2> $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$n-$a.err &
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
