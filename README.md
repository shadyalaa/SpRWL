## Speculative Read-Write Locks ##

**Requirements:**

* Install: [tcmalloc](http://goog-perftools.sourceforge.net/doc/tcmalloc.html) (needed for all backends) and [boost](https://www.boost.org/) (needed for TPC-C).

This source code has three directories: 
1. *backends*: impelementation of different synchronization techniques and 
2. *benchmarks*: the benchmarks used in the experiments of the paper.
3. *evaluation*: scripts used to launch experiments of the paper.

**backends:**

* 1.0-htm-sgl: plain HTM (TLE)
* 10.02-RHTMNW: SpRWL without scheduling (NoSched)
* 10.04-RHTMRW: SpRWL with readers waiting for writers to finish but do not start together (RWait)
* 10.05-RHTMRS: SpRWL with readers waiting for writers and attempting to start together (RSync)
* 10.1-RHTMO: SpRWL with readers' and writers' syncrhonization (SpRWL)
* 10.2-RHTMS: SPRWL with SNZI used for readers' indicator (SNZI)
* herwl: Implemenetation of Hardware Read-Write Lock Elision (RW-LE) *works only on POWER8 architecture*
* rwl: read-write lock of pthreads (RWL)
* brlock: Implementation of Big Reader-Writer Lock (BRLock)

**benchmarks:**

* datastructures: Concurrent hash map used as a synthetic benchmark
* tpcc: in-memory port of TPC-C

**evaluation:**

* run-hashmap.sh: runs the experiments for sensitivity analysis (Sec 4.1)
* run-var-comp.sh: runs the experiment for variants compairson (Sec 4.1.1)
* run-rhtmsnzi-comp.sh: runs the experiment for readers' tracking schemes (Sec 4.1.2)
* run-tpcc.sh: runs the experiements for TPC-C (Sec 4.2)

**usage:**

To compile a benchmark, go to its folder under the benchmarks folder and use the "build-XXX.sh" script.
 
These scripts expect the following parameters (in this order):

1. the backend name, corresponding to the folder under "backends"
2. the number of retries for HTM usage
3. the retry policy to adopt in case of HTM capacity aborts, i.e., how the number of retries is updated upon a
   capacity abort, which may be omitted (possible values: 0, do not retry in htm and resort to the fallback path; 1, decrease by
   one; 2, divide by two)
4. *required only for herwl*, the number of retries for ROT usage after falling back from normal HTM 

*NB*: For brlock and rwl the number of retries is not needed.

Hence, a common usage shall be: 
```
bash build-XXX.sh 10.1-RHTMO 10 0
bash build-XXX.sh herwl 10 0 5
bash build-XXX.sh brlock
```

