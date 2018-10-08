#ifndef SPEC_H
#define SPEC_H 1

typedef struct spec_rwlock{
	padded_scalar_l writer_lock;
} spec_rwlock_t;


#define TM_INIT()	TM_INIT_MUT(rwlock)

#define TM_INIT_MUT(lock){\
	lock = (spec_rwlock_t*)malloc(sizeof(spec_rwlock_t));\
        lock->writer_lock.counter = 0;\
}

#define ACQUIRE_WRITE_LOCK(lock) {\
	START_LATENCY();\
	int tries = HTM_RETRIES;\
	local_exec_mode = 0;\
	while (1){\
		while (IS_LOCKED((&(lock->writer_lock)))) {\
			cpu_relax();\
		}\
		START_TRANSACTION();\
		USE_HTM(lock); \
		if (tries <= 0) {\
			USE_GL(lock);\
			START_TRANSACTION();\
			break;\
		}\
	}\
};

#define WRITER_WAIT_FUNCTION(lock)

#define ACQUIRE_READ_LOCK(lock) {\
	ACQUIRE_WRITE_LOCK(lock);\
};

#define RELEASE_READ_LOCK(lock) {\
	UNLOCK(lock);\
	END_RDTSC_R();\
	__sync_synchronize();\
};

#define RELEASE_WRITE_LOCK(lock) {\
        UNLOCK(lock);\
        END_RDTSC_W();\
        __sync_synchronize();\
};

#define UNLOCK(lock){\
	if (!local_exec_mode) {\
		END_HTM();\
		statistics_array[local_thread_id].htm_commits++;\
	}\
	else {\
		RELEASE_LOCK((&(lock->writer_lock)));\
		statistics_array[local_thread_id].lock_commits++;\
	}\
};

#define TM_BEGIN_RO()                 TM_BEGIN(0)
#define TM_EARLY_RELEASE(var)

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
