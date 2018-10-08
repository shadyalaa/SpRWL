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

backends[1]="10.1-RHTMO"
backends[2]="10.2-RHTMS"

benchmarks[1]="hm-u50"

params[1]="-u50 -i300000 -b5000 -r300000 -d1000000 -m"



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
attempts=(1 2 3 4 5)


#for b is the benchmarks, for c backends
cd $1/benchmarks/datastructures
for c in 1 2
do	
	echo "testing "${backends[$c]} >> $resultsdir/desc.txt
	for h in $htm_retries
	do
		for r in $retry_policy
		do
			bash build-datastructures.sh ${backends[$c]} $h $r
			for b in 1 
			do
				for n in 80
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
