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
backends[10]="10.1-RHTMO"
backends[11]="10.2-RHTMS"

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

#benchmarks[6]="reader-hl-u0"
#benchmarks[7]="reader-hl-u10"
#benchmarks[8]="reader-hl-u50"
#benchmarks[9]="reader-hl-u90"
#benchmarks[10]="reader-hl-u100"

#benchmarks[11]="reader-lh-u0"
#benchmarks[12]="reader-lh-u10"
#benchmarks[13]="reader-lh-u50"
#benchmarks[14]="reader-lh-u90"
#benchmarks[15]="reader-lh-u100"

#benchmarks[16]="reader-hh-u0"
#benchmarks[17]="reader-hh-u10"
#benchmarks[18]="reader-hh-u50"
#benchmarks[19]="reader-hh-u90"
#benchmarks[20]="reader-hh-u100"

bStr[1]="hashmap"
bStr[2]="hashmap"
bStr[3]="hashmap"
bStr[4]="hashmap"
bStr[5]="hashmap"

params[1]="-u0 -i300000 -b5000 -r300000 -d1000000 -m1 -a"
params[2]="-u10 -i300000 -b5000 -r300000 -d1000000 -m1 -a"
params[3]="-u50 -i300000 -b5000 -r300000 -d1000000 -m1 -a"
params[4]="-u90 -i300000 -b5000 -r300000 -d1000000 -m1 -a"
params[5]="-u100 -i300000 -b5000 -r300000 -d1000000 -m1 -a"

params[6]="-u0 -i300000 -b5000 -r300000 -d1000000 -m2 -a"
params[7]="-u10 -i300000 -b5000 -r300000 -d1000000 -m2 -a"
params[8]="-u50 -i300000 -b5000 -r300000 -d1000000 -m2 -a"
params[9]="-u90 -i300000 -b5000 -r300000 -d1000000 -m2 -a"
params[10]="-u100 -i300000 -b5000 -r300000 -d1000000 -m2 -a"

params[11]="-u0 -i300000 -b5000 -r300000 -d1000000 -m5 -a"
params[12]="-u10 -i300000 -b5000 -r300000 -d1000000 -m5 -a"
params[13]="-u50 -i300000 -b5000 -r300000 -d1000000 -m5 -a"
params[14]="-u90 -i300000 -b5000 -r300000 -d1000000 -m5 -a"
params[15]="-u100 -i300000 -b5000 -r300000 -d1000000 -m5 -a"

params[16]="-u0 -i300000 -b5000 -r300000 -d1000000 -m10 -a"
params[17]="-u10 -i300000 -b5000 -r300000 -d1000000 -m10 -a"
params[18]="-u50 -i300000 -b5000 -r300000 -d1000000 -m10 -a"
params[19]="-u90 -i300000 -b5000 -r300000 -d1000000 -m10 -a"
params[20]="-u100 -i300000 -b5000 -r300000 -d1000000 -m10 -a"

params[21]="-u0 -i300000 -b5000 -r300000 -d1000000 -m50 -a"
params[22]="-u10 -i300000 -b5000 -r300000 -d1000000 -m50 -a"
params[23]="-u50 -i300000 -b5000 -r300000 -d1000000 -m50 -a"
params[24]="-u90 -i300000 -b5000 -r300000 -d1000000 -m50 -a"
params[25]="-u100 -i300000 -b5000 -r300000 -d1000000 -m50 -a"

params[26]="-u0 -i300000 -b5000 -r300000 -d1000000 -m100 -a"
params[27]="-u10 -i300000 -b5000 -r300000 -d1000000 -m100 -a"
params[28]="-u50 -i300000 -b5000 -r300000 -d1000000 -m100 -a"
params[29]="-u90 -i300000 -b5000 -r300000 -d1000000 -m100 -a"
params[30]="-u100 -i300000 -b5000 -r300000 -d1000000 -m100 -a"

#params[6]="-u0 -i300000 -b5000 -r300000 -d1000000 -m10 -a"
#params[7]="-u10 -i300000 -b5000 -r300000 -d1000000 -m10 -a"
#params[8]="-u50 -i300000 -b5000 -r300000 -d1000000 -m10 -a"
#params[9]="-u90 -i300000 -b5000 -r300000 -d1000000 -m10 -a"
#params[10]="-u100 -i300000 -b5000 -r300000 -d1000000 -m10 -a"

#params[11]="-u0 -b15 -i2400 -r2400 -d1000000 -m1 -a"
#params[12]="-u10 -b15 -i2400 -r2400 -d1000000 -m1 -a"
#params[13]="-u50 -b15 -i2400 -r2400 -d1000000 -m1 -a"
#params[14]="-u90 -b15 -i2400 -r2400 -d1000000 -m1 -a"
#params[15]="-u100 -b15 -i2400 -r2400 -d1000000 -m1 -a"

#params[16]="-u0 -b15 -i2400 -r2400 -d1000000 -m10 -a"
#params[17]="-u10 -b15 -i2400 -r2400 -d1000000 -m10 -a"
#params[18]="-u50 -b15 -i2400 -r2400 -d1000000 -m10 -a"
#params[19]="-u90 -b15 -i2400 -r2400 -d1000000 -m10 -a"
#params[20]="-u100 -b15 -i2400 -r2400 -d1000000 -m10 -a"




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
thread_counts=(1 2 4 8 16 32 64 80)
attempts=(1 2 3 4 5)

#for b is the benchmarks, for c backends
if [ $4 != "no_random" ];then
echo "common policies: " >> $resultsdir/desc.txt
echo "retries used: "$htm_retries >> $resultsdir/desc.txt
echo "retry policies used: "$retry_policy >> $resultsdir/desc.txt
echo "threads tested: "$thread_counts >> $resultsdir/desc.txt
echo "attempts: "${#alphas[@]} >> $resultsdir/desc.txt
echo "benchmarks parameters:"	>> $resultsdir/desc.txt
for b in {1..20}
do
	echo ${benchmarks[$b]} "=" ${params[$b]} >> $resultsdir/desc.txt
done
fi

cd benchmarks/datastructures
for c in 6 10 11 12 13 14 #1 2 3 4 11 12 13
do	
	echo "testing "${backends[$c]} >> $resultsdir/desc.txt
	for h in $htm_retries
	do
		for r in $retry_policy
		do
			bash build-datastructures.sh ${backends[$c]} $h $r
			for b in {1..20}
			do
				for n in 1 2 4 8 14 28 56
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

>>>>>>> 52178489f56f0394528621587ac69f985b854628
alphas=(100000)
for c in 10 11
do
	echo "testing "${backends[$c]}" with alphas: ""${alphas[@]}">> $resultsdir/desc.txt
	for h in $htm_retries
	do
		for r in $retry_policy
		do
			bash build-datastructures.sh ${backends[$c]} $h $r
			for b in 22 23 24 27 28 29
			do
				for n in 1 2 4 8 16 32 64 80
				do
					#for t in "${alphas[@]}"
					#do
						for a in {1..5}
						do
			    				echo "${benchmarks[$b]} | ${backends[$c]}-$h-$r | threads $n | attempt $a"
				    			./hashmap/hashmap ${params[$b]} -n $n > $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$n-$a.data 2> $runsdir/${benchmarks[$b]}-${backends[$c]}-$h-$r-$n-$a.err &
					    		pid3=$!
								wait_until_finish $pid3
								wait $pid3
		    					rc=$?
					    		if [[ $rc != 0 ]] ; then
	    							echo "Error within: | ${benchmarks[$b]} | ${backends[$c]}-$h-$r | attempt $a" >> $runsdir/error.out
					    		fi
						done
					#done
				done
			done
		done
	done
done



exit 0
