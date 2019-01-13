#ifndef SPEC_H
#define SPEC_H 1
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

# define ACQUIRE_READ_LOCK(lock){ \
	START_LATENCY();\
	while(1){ \
		while(IS_LOCKED((&(lock->reader_locks[local_thread_id])))){ \
                	cpu_relax(); \
                } \
		if(__sync_val_compare_and_swap(&(lock->reader_locks[local_thread_id].counter), 0, 1) == 0){ \
			break; \
		}\
	} \
	START_TRANSACTION();\
}; \

# define RELEASE_READ_LOCK(lock) lock->reader_locks[local_thread_id].counter = 0; statistics_array[local_thread_id].lock_commits++; END_RDTSC_R();\

# define ACQUIRE_WRITE_LOCK(lock) { \
	START_LATENCY();\
	long index;\
        while(1){ \
                while(IS_LOCKED((&(lock->writer_lock)))){ \
                        cpu_relax(); \
                } \
		if(__sync_val_compare_and_swap(&(lock->writer_lock.counter), 0, 1) == 0){ \
                        break; \
                } \
        } \
	for(index=0; index < local_num_threads; index++){ \
		while(1){ \
	                while(IS_LOCKED((&(lock->reader_locks[index])))){ \
                	        cpu_relax(); \
        	        } \
			if(__sync_val_compare_and_swap(&(lock->reader_locks[index].counter), 0, 1) == 0){ \
                        	break; \
                	} \
        	} \
	}\
	START_TRANSACTION();\
};

# define RELEASE_WRITE_LOCK(lock){ \
        long index;\
	for(index=0; index < local_num_threads; index++){ \
                 lock->reader_locks[index].counter = 0;\
        }\
	lock->writer_lock.counter = 0;\
	statistics_array[local_thread_id].lock_commits++; \
	END_RDTSC_W();\
};

#endif
