#ifndef SPEC_H
#define SPEC_H 1


# define IS_GLOBAL_LOCKED(lock)        ((volatile long)(lock->counter) == 2)


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

# define ACQUIRE_READ_LOCK(lock) { \
				START_LATENCY(); \
				while(1){ \
								while(IS_GLOBAL_LOCKED((&((lock)->writer_lock)))){ \
												cpu_relax(); \
								} \
								(lock)->reader_locks[local_thread_id].counter++; \
								__sync_synchronize();\
								if(IS_GLOBAL_LOCKED((&((lock)->writer_lock)))){ \
												(lock)->reader_locks[local_thread_id].counter++; \
								} \
								else{ \
												break; \
								} \
				} \
				START_TRANSACTION(); \
};\

# define RELEASE_READ_LOCK(lock) READ_WRITE_BARRIER(); (lock)->reader_locks[local_thread_id].counter++; END_RDTSC_R(); statistics_array[local_thread_id].uro_commits++;


# define USE_ROT(lock){ \
								while(1){ \
									while(IS_LOCKED((&((lock)->writer_lock)))){ \
												cpu_relax(); \
									} \
									if(__sync_val_compare_and_swap(&((lock)->writer_lock.counter), 0, 1) == 0){\
												break; \
									} \
								} \
								while(tries>0){ \
												TM_buff_type TM_buff; \
												unsigned char tx_status = __TM_begin_rot(&TM_buff); \
												if (tx_status == _HTM_TBEGIN_STARTED) { \
																break; \
												} \
												else if(__TM_is_conflict(&TM_buff)){ \
																statistics_array[local_thread_id].rot_conflict_aborts ++; \
																if(__TM_is_self_conflict(&TM_buff)) statistics_array[local_thread_id].rot_self_conflicts++; \
																else if(__TM_is_trans_conflict(&TM_buff)) statistics_array[local_thread_id].rot_trans_conflicts++; \
																else if(__TM_is_nontrans_conflict(&TM_buff)) statistics_array[local_thread_id].rot_nontrans_conflicts++; \
																tries--; \
												} \
												else if (__TM_is_user_abort(&TM_buff)) { \
																statistics_array[local_thread_id].rot_explicit_aborts ++; \
																tries--; \
												} \
												else if(__TM_is_footprint_exceeded(&TM_buff)){ \
																SPEND_BUDGET(&tries);\
																statistics_array[local_thread_id].rot_capacity_aborts ++; \
																if(__TM_is_failure_persistent(&TM_buff)) statistics_array[local_thread_id].rot_persistent_aborts ++; \
												} \
												else{ \
																tries--; \
																statistics_array[local_thread_id].rot_other_aborts ++; \
												} \
								} \
				};

# define ACQUIRE_GLOBAL_LOCK(lock){ \
				(lock)->writer_lock.counter = 2; \
				__sync_synchronize(); \
				QUIESCENCE_CALL_GL(lock); \
};


# define WRITER_WAIT_FUNCTION(a)


#define ACQUIRE_WRITE_LOCK(lock) {\
  local_exec_mode = 0;\
  START_LATENCY();\
	int tries = HTM_RETRIES;\
  while (tries > 0){\
    while (IS_LOCKED((&(lock->writer_lock)))){\
      cpu_relax(); \
    }\
    START_TRANSACTION();\
    USE_HTM(lock); \
	} \
  if (tries <= 0) {\
		local_exec_mode = 1; \
    tries = ROT_RETRIES; \
    USE_ROT(lock); \
    if(tries <= 0){ \
			local_exec_mode = 2; \
      ACQUIRE_GLOBAL_LOCK(lock); \
    }\
  }\
}


# define QUIESCENCE_CALL(lock){ \
				long index;\
				volatile long temp; \
				long counters_snapshot[local_num_threads]; \
				for(index=0; index < local_num_threads; index++){ \
								temp = (lock)->reader_locks[index].counter; \
								if((temp & 1) != 0){ \
												counters_snapshot[index] = temp; \
								}\
								else{ \
												counters_snapshot[index] = 0; \
								} \
				} \
				for(index=0; index < local_num_threads; index++){ \
								if(counters_snapshot[index] != 0){ \
												while((lock)->reader_locks[index].counter == counters_snapshot[index]){ \
																cpu_relax(); \
												} \
								} \
				} \
};

# define QUIESCENCE_CALL_GL(lock){ \
				/*__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t index; */ int index;\
				for(index=0; index < local_num_threads; index++){ \
								while(((lock)->reader_locks[index].counter & 1) != 0){ \
												cpu_relax(); \
								} \
				} \
};

# define RELEASE_WRITE_LOCK(lock){ \
				if(local_exec_mode == 0){ \
								__TM_suspend(); \
								QUIESCENCE_CALL_GL(lock); \
								__TM_resume(); \
								__TM_end(); \
								statistics_array[local_thread_id].htm_commits++; \
				} \
				else if(local_exec_mode == 1){ \
								QUIESCENCE_CALL(lock); \
								__TM_end(); \
								(lock)->writer_lock.counter = 0;\
								statistics_array[local_thread_id].rot_commits++; \
				} \
				else{ \
								(lock)->writer_lock.counter = 0;\
								statistics_array[local_thread_id].lock_commits++; \
				} \
				END_RDTSC_W(); \
};

#endif
