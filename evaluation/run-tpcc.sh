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

backends[1]="1.0-htm-sgl"       #does not use alpha
backends[2]="2.0-NoWait"        #does not use alpha
backends[3]="brlock"            #does not use alpha
backends[4]="rwl"               #does not use alpha
backends[5]="herwl"
backends[6]="5.75-RWSO"         #does not use alpha
backends[7]="10.1-RHTMO"
backends[8]="10.2-RHTMS"
backends[9]="5.65-RWS"

benchmarks[1]="tpcc-hh-sp-u1"
benchmarks[2]="tpcc-hh-sp-u10"
benchmarks[3]="tpcc-hh-sp-u50"

benchmarks[4]="tpcc-baln"
benchmarks[5]="tpcc-stn"

params[1]="-s 99 -d 0 -o 0 -p 1 -r 0"
params[2]="-s 90 -d 0 -o 0 -p 10 -r 0"
params[3]="-s 50 -d 0 -o 0 -p 50 -r 0"

#balanced params
params[4]="-s 31 -d 2 -o 4 -p 43 -r 20"
params[5]="-s 31 -d 4 -o 4 -p 43 -r 18"


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

#for backends 1-3 do only alpha=1
#for backend 4-5 do alpha=500,1000,100000(WaitAll)
#for backend 5-6 do alpha= 5000, 10000, 50000

htm_retries=10
retry_policy=0
rot_retry=5
if cat /proc/cpuinfo | grep -q "Intel"; then
	thread_counts=( 1 2 4 8 14 28 42 56)
else
	thread_counts=( 1 2 4 8 16 32 64 80)
fi
attempts=(1 2 3 4 5)

#for b is the benchmarks, for c backends
if [ $4 != "no_random" ];then
echo "common policies: " >> $resultsdir/desc.txt
echo "retries used: "$htm_retries >> $resultsdir/desc.txt
echo "retry policies used: "$retry_policy >> $resultsdir/desc.txt
echo "threads tested: "$thread_counts >> $resultsdir/desc.txt
echo "attempts: "${#alphas[@]} >> $resultsdir/desc.txt
fi

cd $1/benchmarks/tpcc
for c in 4 #1 2 3 4 5 7 8
do
	attempts=(1 2 3 4 5)
	if [[ $c == 8 ]] ; then
            attempts=(1 2 3 4 5 6 7 8 9 10)
  	fi
	echo "testing "${backends[$c]}" with alphas: ""${alphas[@]}">> $resultsdir/desc.txt
	for h in $htm_retries
	do
		for r in $retry_policy
		do
			bash build-tpcc.sh ${backends[$c]} $h $r $rot_retry
			for b in 5
			do
				for n in "${thread_counts[@]}"
				do
						for a in "${attempts[@]}"
						do
							echo "${benchmarks[$b]} | ${backends[$c]}-$h-$r | threads $n | attempt $a"
							exec="-w $n -m $n"
				    			./code/tpcc -t 5 $exec ${params[$b]} -n $n > $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$n-$a.data 2> $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$n-$a.err &
					    		pid3=$!
								#wait_until_finish $pid3
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
