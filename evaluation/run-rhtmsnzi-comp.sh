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

backends[1]="1.0-htm-sgl"       #does not use alpha
backends[2]="2.0-NoWait"        #does not use alpha
backends[3]="brlock"            #does not use alpha
backends[4]="rwl"               #does not use alpha
backends[5]="herwl"
backends[10]="10.1-RHTMO"
backends[11]="10.2-RHTMS"

benchmarks[1]="hm-u0"
benchmarks[2]="hm-u10"
benchmarks[3]="hm-u50"



params[1]="-u0 -i100000 -b5000 -r100000 -d1000000 -m"
params[2]="-u10 -i100000 -b5000 -r100000 -d1000000 -m"
params[3]="-u50 -i100000 -b5000 -r100000 -d1000000 -m"



benchmarks[4]="hm10k-u0"
benchmarks[5]="hm10k-u10"

params[4]="-u0 -i10000 -b5000 -r10000 -d5000000 -m"
params[5]="-u10 -i10000 -b5000 -r10000 -d5000000 -m"


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
thread_counts=( 8 14 28 56)
attempts=(1 2 3 4 5)

#for b is the benchmarks, for c backends
if [ $4 != "no_random" ];then
echo "common policies: " >> $resultsdir/desc.txt
echo "retries used: "$htm_retries >> $resultsdir/desc.txt
echo "retry policies used: "$retry_policy >> $resultsdir/desc.txt
echo "threads tested: "$thread_counts >> $resultsdir/desc.txt
echo "attempts: "${#alphas[@]} >> $resultsdir/desc.txt
echo "benchmarks parameters:"	>> $resultsdir/desc.txt
fi

#for b is the benchmarks, for c backends
cd $1/benchmarks/datastructures
for c in 10 11 #1 2 3 4 11 12 13
do	
	echo "testing "${backends[$c]} >> $resultsdir/desc.txt
	for h in $htm_retries
	do
		for r in $retry_policy
		do
			bash build-datastructures.sh ${backends[$c]} $h $r
			for b in 1 2 3
			do
				for n in 10 80
				do
					for m in 1 2 5 10 50 100 500 1000 10000
					do
						for a in {1..5}
						do
		    					echo "${benchmarks[$b]} | ${backends[$c]}-$h-$r | threads $n| attempt $a | reader size $m"
			    				./hashmap/hashmap ${params[$b]} $m -n $n > $runsdir/${benchmarks[$b]}$n-${backends[$c]}-$h-$r-$m-$a.data 2> $runsdir/${benchmarks[$b]}$n-${backends[$c]}-$h-$r-$m-$a.err &
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
done

exit 0
