#ifndef SPEC_H
#define SPEC_H 1
#define NON_VARIANT

typedef struct spec_rwlock{
        __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_l* reader_locks;
        __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_l writer_lock;
} spec_rwlock_t;

#define TM_INIT() TM_INIT_MUT(rwlock)

#define TM_INIT_MUT(lock){\
        lock = (spec_rwlock_t*) malloc(sizeof(spec_rwlock_t));\
        lock->writer_lock.counter = 0;\
        lock->reader_locks = (padded_scalar_l*)malloc(NR_CPUS*sizeof(padded_scalar_l));\
        for(int i = 0; i < NR_CPUS; i++){\
                lock->reader_locks[i].counter = 0;\
        }\
}

#define ACQUIRE_WRITE_LOCK(lock) {\
	int tries = HTM_RETRIES;\
	local_exec_mode = 0;\
	START_LATENCY();\
	while (1){\
		while (IS_LOCKED((&(lock->writer_lock)))){\
			cpu_relax(); \
		}\
		START_TRANSACTION();\
		USE_HTM(lock); \
		if (tries <= 0) {\
			USE_GL(lock);\
			int index = 0;\
			for(index=0; index < local_num_threads; index++){\
				while(lock->reader_locks[index].counter) cpu_relax();\
			}\
			START_TRANSACTION();\
			break;\
		}\
	}\
};

#define WRITER_WAIT_FUNCTION(lock)

#define ACQUIRE_READ_LOCK(lock) {\
	START_LATENCY();\
	while(1){\
		lock->reader_locks[local_thread_id].counter = 1;\
		__sync_synchronize();\
		if(IS_LOCKED((&(lock->writer_lock)))){\
			lock->reader_locks[local_thread_id].counter = 0;\
			while(IS_LOCKED((&(lock->writer_lock)))){\
				cpu_relax(); \
			}\
			continue;\
		}\
		break;\
	}\
	START_TRANSACTION();\
};

#define RELEASE_WRITE_LOCK(lock){\
	if (!local_exec_mode) {\
		int index;\
		for(index=0; index < local_num_threads; index++){\
			if(lock->reader_locks[index].counter){\
				ABORT_HTM(0xa0);\
			}\
		}\
		END_HTM();\
		statistics_array[local_thread_id].htm_commits++;\
	}\
	else {\
		RELEASE_LOCK((&(lock->writer_lock)));\
		statistics_array[local_thread_id].lock_commits++;\
	}\
	END_RDTSC_W();\
	__sync_synchronize();\
};

#define TM_BEGIN_RO()                 TM_BEGIN(0)
#define TM_RESTART()                  _xabort(0xab);
#define TM_EARLY_RELEASE(var)

#define FAST_PATH_RESTART() _xabort(0xab);
#define FAST_PATH_SHARED_READ(var) (var)
#define FAST_PATH_SHARED_READ_P(var) (var)
#define FAST_PATH_SHARED_READ_D(var) (var)
#define FAST_PATH_SHARED_WRITE(var, val) ({var = val; var;})
#define FAST_PATH_SHARED_WRITE_P(var, val) ({var = val; var;})
#define FAST_PATH_SHARED_WRITE_D(var, val) ({var = val; var;})

#define SLOW_PATH_RESTART() FAST_PATH_RESTART()
#define SLOW_PATH_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
#define SLOW_PATH_SHARED_READ_P(var)         FAST_PATH_SHARED_READ_P(var)
#define SLOW_PATH_SHARED_READ_F(var)         FAST_PATH_SHARED_READ_D(var)
#define SLOW_PATH_SHARED_READ_D(var)         FAST_PATH_SHARED_READ_D(var)
#define SLOW_PATH_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
#define SLOW_PATH_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
#define SLOW_PATH_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)

#define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#define TM_LOCAL_WRITE_D(var, val)    ({var = val; var;})


#endif
