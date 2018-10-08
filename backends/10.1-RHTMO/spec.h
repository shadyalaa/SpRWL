#ifndef SPEC_H
#define SPEC_H 1
#define WAIT_OFFSET 10000
#define NON_VARIANT

typedef struct spec_rwlock{
				padded_scalar_l writer_lock;
				padded_scalar_l* reader_locks;
				padded_scalar_ll* clocks_w;
				padded_scalar_ll* clocks_r;
				padded_scalar_ll* avg_cycles;
				padded_scalar_ll* is_waiting;
} spec_rwlock_t;

#define TM_INIT() TM_INIT_MUT(rwlock)

#define TM_INIT_MUT(lock){\
				lock = (spec_rwlock_t*) malloc(sizeof(spec_rwlock_t));\
				lock->writer_lock.counter = 0;\
				lock->reader_locks = (padded_scalar_l*)calloc(NR_CPUS,sizeof(padded_scalar_l));\
				lock->clocks_w = (padded_scalar_ll*)calloc(NR_CPUS,sizeof(padded_scalar_ll));\
				lock->clocks_r = (padded_scalar_ll*)calloc(NR_CPUS,sizeof(padded_scalar_ll));\
				lock->avg_cycles = (padded_scalar_ll*)calloc(NR_TYPES,sizeof(padded_scalar_ll));\
				lock->is_waiting = (padded_scalar_ll*)calloc(NR_CPUS,sizeof(padded_scalar_ll));\
}

#define START_RDTSC_R2(lock){\
				statistics_array[local_thread_id].start = rdtscp();\
				lock->clocks_r[local_thread_id].counter = statistics_array[local_thread_id].start + lock->avg_cycles[t_type].counter;\
};

#define END_RDTSC_R2(lock){\
				lock->clocks_r[local_thread_id].counter = 0;\
				__sync_synchronize();\
				unsigned long long end = rdtscp();\
				unsigned long long time = end - statistics_array[local_thread_id].start;\
				if(!local_thread_id){\
								lock->avg_cycles[t_type].counter = ( lock->avg_cycles[t_type].counter * statistics_array[local_thread_id].read_commits + time ) / ( statistics_array[local_thread_id].read_commits + 1);\
				}\
				statistics_array[local_thread_id].r_avg_time+= time;\
				statistics_array[local_thread_id].r_latency += (end - statistics_array[local_thread_id].latency);\
};


#define START_RDTSC_W2(lock){\
				statistics_array[local_thread_id].start = rdtscp();\
				lock->clocks_w[local_thread_id].counter = statistics_array[local_thread_id].start + lock->avg_cycles[t_type].counter;\
};

#define END_RDTSC_W2(lock){\
				lock->clocks_w[local_thread_id].counter = 0;\
				__sync_synchronize();\
				unsigned long long end = rdtscp();\
				unsigned long long time = end - statistics_array[local_thread_id].start;\
				if(!local_thread_id){\
								lock->avg_cycles[t_type].counter = ( lock->avg_cycles[t_type].counter * statistics_array[local_thread_id].write_commits + time ) / ( statistics_array[local_thread_id].write_commits + 1);\
				}\
				statistics_array[local_thread_id].w_avg_time+= time;\
				statistics_array[local_thread_id].w_latency += (end - statistics_array[local_thread_id].latency);\
};

#define READER_WAIT_FUNCTION(lock){\
				long long writer_start = 0;\
				int waited_thread = 0;\
				for(int index = 0 ; index < local_num_threads; index++){\
								int temp = lock->is_waiting[index].counter;\
								if( temp ){\
												writer_start = 1;\
												waited_thread = temp-1;\
												break;\
								}else if( lock->clocks_w[index].counter > writer_start){\
												writer_start = lock->clocks_w[index].counter;\
												waited_thread = index;\
								}\
				}\
				if(writer_start){\
								lock->is_waiting[local_thread_id].counter = waited_thread+1;\
								while( lock->clocks_w[waited_thread].counter ) cpu_relax();\
								lock->is_waiting[local_thread_id].counter = 0 ;\
				}\
};

#define WRITER_WAIT_FUNCTION(lock){\
				long long wait = 0;\
				int waited_thread = 0; \
				for(int index = 0 ; index < local_num_threads; index++){\
								if( lock->clocks_r[index].counter > wait ){\
												wait = lock->clocks_r[index].counter;\
												waited_thread = index ; \
								}\
				}\
				if(wait){\
								wait -= lock->avg_cycles[t_type].counter/2;\
								while(rdtsc() < wait) cpu_relax();\
				}\
};

#define ACQUIRE_WRITE_LOCK(lock) {\
				int tries = HTM_RETRIES;\
				local_exec_mode = 0;\
				START_LATENCY();\
				while (1){\
								while (IS_LOCKED((&(lock->writer_lock)))) {\
												cpu_relax();\
								}\
								START_RDTSC_W2(lock); \
								USE_HTM(lock); \
								if (tries <= 0) {\
												USE_GL(lock);\
												int index = 0;\
												for(index=0; index < local_num_threads; index++){\
																while(lock->reader_locks[index].counter) cpu_relax();\
												}\
												START_RDTSC_W2(lock); \
												break;\
								}\
				}\
};

#define ACQUIRE_READ_LOCK(lock) {\
				START_LATENCY();\
				int tries = HTM_RETRIES;\
				local_exec_mode = 0;\
				while (1){\
								while (IS_LOCKED((&(lock->writer_lock)))) {\
												cpu_relax();\
								}\
								USE_HTM(lock); \
								if(tries <= 0){\
												READ_MODE(lock);\
												break;\
								}\
				}\
				START_RDTSC_R2(lock);\
}

#define READ_MODE(lock){\
				local_exec_mode = 1;\
				READER_WAIT_FUNCTION(lock);\
				while(1){\
								lock->reader_locks[local_thread_id].counter = 1;\
								__sync_synchronize();\
								if(IS_LOCKED((&(lock->writer_lock)))){\
												lock->reader_locks[local_thread_id].counter = 0;\
												while(IS_LOCKED((&(lock->writer_lock)))){\
																cpu_relax();\
												}\
												continue;\
								}\
								break;\
				}\
};

#define RELEASE_WRITE_LOCK(lock){\
				if (!local_exec_mode) {\
								int index;\
								for(index=0; index < local_num_threads; index++){\
												if(lock->reader_locks[index].counter){\
																ABORT_HTM(90);\
												}\
								}\
								END_HTM();\
								statistics_array[local_thread_id].htm_commits++;\
				}\
				else {\
								RELEASE_LOCK((&(lock->writer_lock)));\
								statistics_array[local_thread_id].lock_commits++;\
				}\
				END_RDTSC_W2(lock);\
};

#undef RELEASE_READ_LOCK

#define RELEASE_READ_LOCK(lock) {\
				if( !local_exec_mode ){\
								END_HTM();\
								statistics_array[local_thread_id].htm_commits++;\
				}else{\
								READ_WRITE_BARRIER(); \
								lock->reader_locks[local_thread_id].counter = 0;\
								statistics_array[local_thread_id].uro_commits++;\
				}\
				END_RDTSC_R2(lock);\
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
