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

backends[1]="10.02-RHTMNW"
backends[2]="10.04-RHTMRW"
backends[3]="10.05-RHTMRS"
backends[4]="10.1-RHTMO"


benchmarks[1]="reader-ll-x10-u10"

params[1]="-u10 -i300000 -b5000 -r300000 -d100000 -m10 -a"

#params[6]="-u0 -i300000 -b5000 -r300000 -d100000 -m10 -a"
#params[7]="-u10 -i300000 -b5000 -r300000 -d100000 -m10 -a"
#params[8]="-u50 -i300000 -b5000 -r300000 -d100000 -m10 -a"
#params[9]="-u90 -i300000 -b5000 -r300000 -d100000 -m10 -a"
#params[10]="-u100 -i300000 -b5000 -r300000 -d100000 -m10 -a"

#params[11]="-u0 -b15 -i2400 -r2400 -d100000 -m1 -a"
#params[12]="-u10 -b15 -i2400 -r2400 -d100000 -m1 -a"
#params[13]="-u50 -b15 -i2400 -r2400 -d100000 -m1 -a"
#params[14]="-u90 -b15 -i2400 -r2400 -d100000 -m1 -a"
#params[15]="-u100 -b15 -i2400 -r2400 -d100000 -m1 -a"

#params[16]="-u0 -b15 -i2400 -r2400 -d100000 -m10 -a"
#params[17]="-u10 -b15 -i2400 -r2400 -d100000 -m10 -a"
#params[18]="-u50 -b15 -i2400 -r2400 -d100000 -m10 -a"
#params[19]="-u90 -b15 -i2400 -r2400 -d100000 -m10 -a"
#params[20]="-u100 -b15 -i2400 -r2400 -d100000 -m10 -a"




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
attempts=(1 2 3 4 5)

#for b is the benchmarks, for c backends
if [ $4 != "no_random" ];then
echo "common policies: " >> $resultsdir/desc.txt
echo "retries used: "$htm_retries >> $resultsdir/desc.txt
echo "retry policies used: "$retry_policy >> $resultsdir/desc.txt
echo "attempts: "${#alphas[@]} >> $resultsdir/desc.txt
echo "benchmarks parameters:"	>> $resultsdir/desc.txt
for b in 1
do
	echo ${benchmarks[$b]} "=" ${params[$b]} >> $resultsdir/desc.txt
done
fi

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
				for n in 1 2 4 8 16 32 64 80
				do
					for a in {1..5}
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
