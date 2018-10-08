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

backends[1]="10.02-RHTMNW" # NoSched
backends[2]="10.04-RHTMRW" # RWait
backends[3]="10.05-RHTMRS" # RSync
backends[4]="10.1-RHTMO" #Sprwl


benchmarks[1]="reader-ll-x10-u10"

params[1]="-u10 -i$items -b5000 -r$items -d100000 -m10 -a"


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

#for b is the benchmarks, for c backends
cd $1/benchmarks/datastructures
for c in 1 2 3 4
do	
	echo "testing "${backends[$c]} >> $resultsdir/desc.txt
	for h in $htm_retries
	do
		for r in $retry_policy
		do
			bash build-datastructures.sh ${backends[$c]} $h $r
			for b in 1
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
