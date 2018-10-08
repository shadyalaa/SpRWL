#ifndef TM_H
#define TM_H 1

#include "common_types.h"
#include "spec.h"
#include <stdio.h>
#include "thread.h"

#ifndef REDUCED_TM_API

#define MAIN(argc, argv)              int main (int argc, char** argv)
#define MAIN_RETURN(val)              return val

#define GOTO_SIM()                    /* nothing */
#define GOTO_REAL()                  /* nothing */
#define IS_IN_SIM()                   (0)

#define SIM_GET_NUM_CPU(var)          /* nothing */

#define TM_PRINTF                     printf
#define TM_PRINT0                     printf
#define TM_PRINT1                     printf
#define TM_PRINT2                     printf
#define TM_PRINT3                     printf

#define P_MEMORY_STARTUP(numThread)   /* nothing */
#define P_MEMORY_SHUTDOWN()           /* nothing */

#include "memory.h"
#include "types.h"
#include <math.h>

#endif


#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__) /* Power8 */
#include <htmxlintrin.h>
#else /* intel14 */
#include <immintrin.h>
#include <rtmintrin.h>
#endif

#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_self_conflict(void* const TM_buff)
{
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_SELF_INDUCED_CONFLICT (texasr);
}
extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_trans_conflict(void* const TM_buff)
{
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_TRANSACTION_CONFLICT (texasr);
}
extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_nontrans_conflict(void* const TM_buff)
{
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_NON_TRANSACTIONAL_CONFLICT (texasr);
}
extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_begin_rot (void* const TM_buff)
{
  *_TEXASRL_PTR (TM_buff) = 0;
  if (__builtin_expect (__builtin_tbegin (1), 1)){
    return _HTM_TBEGIN_STARTED;
  }
#ifdef __powerpc64__
  *_TEXASR_PTR (TM_buff) = __builtin_get_texasr ();
#else
  *_TEXASRU_PTR (TM_buff) = __builtin_get_texasru ();
  *_TEXASRL_PTR (TM_buff) = __builtin_get_texasr ();
#endif
  *_TFIAR_PTR (TM_buff) = __builtin_get_tfiar ();
  return 0;
}


#define  rdtsc(void)({ \
	unsigned long long int result=0;\
	unsigned long int upper, lower,tmp;\
	__asm__ volatile(\
			"0:                  \n"\
			"\tmftbu   %0           \n"\
			"\tmftb    %1           \n"\
			"\tmftbu   %2           \n"\
			"\tcmpw    %2,%0        \n"\
			"\tbne     0b         \n"\
			: "=r"(upper),"=r"(lower),"=r"(tmp)\
			);\
	result = upper;\
	result = result<<32;\
	result = result|lower;\
	result;\
})

#define rdtscp(void) rdtsc(void)

#define cpu_relax()  asm volatile ("" ::: "memory");

#else

#define cpu_relax() _mm_pause();

#define rdtscp(void) ({ \
		register unsigned long long res; \
		asm volatile ( \
				"xor %%rax,%%rax \n\t" \
				"rdtsc          \n\t" \
				"shl $32,%%rdx   \n\t" \
				"or  %%rax,%%rdx \n\t" \
				"mov %%rdx,%0" \
				: "=r"(res) \
				: \
				: "rax", "rdx"); \
		res; \
		})

#define rdtsc(void) ({ \
		register unsigned long long res; \
		asm volatile ( \
				"xor %%rax,%%rax \n\t" \
				"rdtsc          \n\t" \
				"shl $32,%%rdx   \n\t" \
				"or  %%rax,%%rdx \n\t" \
				"mov %%rdx,%0" \
				: "=r"(res) \
				: \
				: "rax", "rdx"); \
		res; \
		})
/*
   static __inline__ volatile unsigned long long rdtscp(void)
   {
   unsigned hi, lo;
   __asm__ __volatile__ ("rdtscp" : "=a"(lo), "=d"(hi));
   return (volatile) ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
   }

   static __inline__ volatile unsigned long long rdtsc(void)
   {
   unsigned hi, lo;
   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
   return (volatile) ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
   }
 */
#endif

#define TM_ARG                        /* nothing */
#define TM_ARG_ALONE                  /* nothing */
#define TM_ARGDECL                    /* nothing */
#define TM_ARGDECL_ALONE              /* nothing */
#define TM_CALLABLE                   /* nothing */

#define TM_BEGIN_WAIVER()
#define TM_END_WAIVER()

#define P_MALLOC(size)                malloc(size)
#define P_FREE(ptr)                   free(ptr)
#define TM_MALLOC(size)               malloc(size)
#define FAST_PATH_FREE(ptr)            free(ptr)
#define SLOW_PATH_FREE(ptr)             free(ptr)

#define SETUP_NUMBER_TASKS(n)
#define SETUP_NUMBER_THREADS(n)
#define PRINT_STATS()
#define AL_LOCK(idx)

//#endif

#ifdef REDUCED_TM_API
#define SPECIAL_THREAD_ID()         local_thread_id
#elif KYOTO
#define SPECIAL_THREAD_ID()	local_thread_id
#else
#define SPECIAL_THREAD_ID()         thread_getId()
#endif

#define TM_STARTUP(numthreads,bid) TM_INIT()

#define TM_SHUTDOWN() {\
	unsigned long write_commits = 0;\
	unsigned long read_commits = 0;\
	unsigned long aborts = 0;\
	unsigned long rot_aborts = 0; \
	unsigned long htm_aborts = 0; \
	unsigned long capacity_aborts = 0;\
	unsigned long conflict_aborts = 0;\
	unsigned long explicit_aborts = 0;\
	unsigned long retry_aborts = 0;\
	unsigned long nested_aborts = 0;\
	unsigned long other_aborts = 0;\
	unsigned long trans_conflicts = 0;\
  unsigned long nontrans_conflicts = 0;\
  unsigned long self_conflicts = 0;\
  unsigned long readers_aborts = 0;\
	unsigned long rot_capacity_aborts = 0;\
  unsigned long rot_conflict_aborts = 0;\
  unsigned long rot_explicit_aborts = 0;\
  unsigned long rot_persistent_aborts = 0;\
  unsigned long rot_other_aborts = 0;\
  unsigned long rot_trans_conflicts = 0;\
  unsigned long rot_nontrans_conflicts = 0;\
  unsigned long rot_self_conflicts = 0;\
  unsigned long rot_readers_aborts = 0;\
	unsigned long long avg_reader_time = 0;\
	unsigned long long avg_writer_time = 0;\
	unsigned long long avg_reader_latency = 0;\
	unsigned long long avg_writer_latency = 0;\
	unsigned long lock_commits = 0;\
	unsigned long htm_commits = 0;\
	unsigned long uro_commits= 0;\
	unsigned long rot_commits= 0;\
	unsigned long waited_threads = 0;\
	int i = 0;\
	if(!local_num_threads)	local_num_threads = NR_CPUS;\
	for (i = 0; i < local_num_threads; i++) {\
		readers_aborts += statistics_array[i].readers_aborts;\
		trans_conflicts += statistics_array[i].trans_conflicts;\
		nontrans_conflicts += statistics_array[i].nontrans_conflicts;\
		self_conflicts += statistics_array[i].self_conflicts;\
		explicit_aborts += statistics_array[i].explicit_aborts;\
		capacity_aborts += statistics_array[i].capacity_aborts;\
    conflict_aborts += statistics_array[i].conflict_aborts;\
		retry_aborts += statistics_array[i].retry_aborts;\
    nested_aborts += statistics_array[i].nested_aborts;\
    other_aborts += statistics_array[i].other_aborts;\
		rot_trans_conflicts += statistics_array[i].rot_trans_conflicts;\
    rot_nontrans_conflicts += statistics_array[i].rot_nontrans_conflicts;\
    rot_self_conflicts += statistics_array[i].rot_self_conflicts;\
    rot_explicit_aborts += statistics_array[i].rot_explicit_aborts;\
    rot_capacity_aborts += statistics_array[i].rot_capacity_aborts;\
    rot_conflict_aborts += statistics_array[i].rot_conflict_aborts;\
    rot_persistent_aborts += statistics_array[i].rot_persistent_aborts;\
    rot_other_aborts += statistics_array[i].rot_other_aborts;\
		htm_commits += statistics_array[i].htm_commits;\
		lock_commits += statistics_array[i].lock_commits;\
		read_commits += statistics_array[i].read_commits;\
		write_commits += statistics_array[i].write_commits;\
		other_aborts += statistics_array[i].other_aborts;\
		avg_reader_time += statistics_array[i].r_avg_time;\
		avg_writer_time += statistics_array[i].w_avg_time;\
		waited_threads += statistics_array[i].w_threads;\
		avg_writer_latency += statistics_array[i].w_latency;\
		avg_reader_latency += statistics_array[i].r_latency;\
		uro_commits += statistics_array[i].uro_commits;\
		rot_commits += statistics_array[i].rot_commits;\
	}\
	if( read_commits){\
		avg_reader_time = avg_reader_time / read_commits;\
		avg_reader_latency = avg_reader_latency / read_commits;\
	}\
	if( write_commits){\
		avg_writer_time = avg_writer_time / write_commits;\
		avg_writer_latency = avg_writer_latency / write_commits;\
	}\
	htm_aborts = capacity_aborts + conflict_aborts + explicit_aborts + retry_aborts + nested_aborts + other_aborts + readers_aborts ;\
	rot_aborts = rot_capacity_aborts + rot_conflict_aborts + rot_explicit_aborts + rot_other_aborts + rot_readers_aborts ;\
	aborts = htm_aborts + rot_aborts; \
	printf("threads: %d processes\n", local_num_threads);\
  printf("Total read transactions: %lu\n",read_commits);\
  		printf("Total write transactions: %lu\n",write_commits);\
			printf("\t\tCapacity Aborts: %lu\n",capacity_aborts);\
			printf("\t\t\tTrans aborts: %lu\n",trans_conflicts);\
			printf("\t\t\tNontrans aborts: %lu\n",nontrans_conflicts);\
			printf("\t\t\tSelf aborts: %lu\n",self_conflicts);\
			printf("\t\tConflict aborts: %lu\n",conflict_aborts);\
			printf("\t\tReaders aborts: %lu\n",readers_aborts);\
			printf("\t\tExplicit aborts: %lu\n",explicit_aborts);\
			printf("\t\tRetry aborts: %lu\n",retry_aborts);\
			printf("\t\tNested aborts: %lu\n",nested_aborts);\
			printf("\t\tOther aborts: %lu\n",other_aborts);\
		printf("\tHTM aborts: %lu\n",htm_aborts);\
			printf("\t\t\tROT Persistent aborts: %lu\n",rot_persistent_aborts);\
      printf("\t\tROT Capacity Aborts: %lu\n",rot_capacity_aborts);\
      printf("\t\t\tROT Trans aborts: %lu\n",rot_trans_conflicts);\
      printf("\t\t\tROT Nontrans aborts: %lu\n",rot_nontrans_conflicts);\
      printf("\t\t\tROT Self aborts: %lu\n",rot_self_conflicts);\
      printf("\t\tROT Conflict aborts: %lu\n",rot_conflict_aborts);\
      printf("\t\tROT Readers aborts: %lu\n",rot_readers_aborts);\
      printf("\t\tROT Explicit aborts: %lu\n",rot_explicit_aborts);\
      printf("\t\tROT Other aborts: %lu\n",rot_other_aborts);\
  	printf("\tROT aborts: %lu\n",rot_aborts);\
	printf("Total aborts: %lu\n",aborts);\
	printf("\tHTM Commits: %lu\n",htm_commits);\
	printf("\tROT Commits: %lu\n",rot_commits);\
	printf("\tLock Commits: %lu\n",lock_commits);\
	printf("\tURO Commits: %lu\n",uro_commits);\
	printf("Total commits: %lu\n",write_commits+read_commits);\
	printf("Avg Writer Time: %lu Cycles\n",avg_writer_time);\
	printf("Avg Reader Time: %lu Cycles\n",avg_reader_time);\
	printf("Latency Writer: %lu\n",avg_writer_latency);\
	printf("Latency Reader: %lu\n",avg_reader_latency);\
	printf("Alpha: %lu\n",alpha);\
	printf("Waited: %lu\n",waited_threads);\
};

#define TM_THREAD_ENTER() 
#define TM_THREAD_EXIT()

#define IS_LOCKED(lock)        ((volatile long)(lock->counter) != 0)

#define SPEND_BUDGET(b)	if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

#define RELEASE_LOCK(lock) lock->counter = 0;

#define USE_GL(mutex){\
	local_exec_mode = 2;\
	while (__sync_val_compare_and_swap((&(mutex->writer_lock.counter)), 0, 1) == 1) {\
		cpu_relax();\
	}\
};

#define TM_BEGIN(b) TM_BEGIN_MUT(b, 0, rwlock);

#define TM_BEGIN_EXT(b, ro) TM_BEGIN_MUT(b, ro , rwlock);

#define TM_BEGIN_MUT(b, ro, mutex) {\
	t_type = b;\
	if(ro) {\
		ACQUIRE_READ_LOCK(mutex);\
	}\
	else {\
		ACQUIRE_WRITE_LOCK(mutex);\
	}\
};

#define TM_END() TM_END_MUT(rwlock);

#define TM_END_MUT(mutex){\
	if(ro){\
		RELEASE_READ_LOCK(mutex);\
		statistics_array[local_thread_id].read_commits++;\
	}\
	else{\
		RELEASE_WRITE_LOCK(mutex);\
		statistics_array[local_thread_id].write_commits++;\
	}\
};

/*******		Statistic Specific		********/
#define START_LATENCY(){\
	statistics_array[local_thread_id].latency = rdtsc();\
}

#define START_TRANSACTION() {\
	statistics_array[local_thread_id].start = rdtsc();\
};

#define END_RDTSC_R(){\
	unsigned long long end = rdtsc();\
	statistics_array[local_thread_id].r_avg_time+= (end - statistics_array[local_thread_id].start);\
	statistics_array[local_thread_id].r_latency += (end - statistics_array[local_thread_id].latency);\
};

#define END_RDTSC_W(){\
	unsigned long long end = rdtsc();\
	statistics_array[local_thread_id].w_avg_time+= (end - statistics_array[local_thread_id].start);\
	statistics_array[local_thread_id].w_latency += (end - statistics_array[local_thread_id].latency);\
};
/*******		Statistic Specific		********/

/*******		Writer Active Mark		********/
#define FLAG_WRITER(lock){\
	lock->clocks[local_thread_id].counter = 1;\
	START_TRANSACTION();\
};

#define UNFLAG_WRITER(lock){\
	lock->clocks[local_thread_id].counter = 0;\
	END_RDTSC_W();\
};
/*******		Writer Active Mark		********/

#ifdef NON_VARIANT
#define RELEASE_READ_LOCK(lock) {\
  READ_WRITE_BARRIER(); \
  lock->reader_locks[local_thread_id].counter = 0;\
  statistics_array[local_thread_id].uro_commits++;\
	END_RDTSC_R();\
};
#endif

#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__) /*Power8*/

#define READ_WRITE_BARRIER() __asm__ volatile ("lwsync")


#define USE_HTM(lock){\
	TM_buff_type TM_buff;\
	unsigned char tx_status = __TM_begin(&TM_buff); \
	char code=0;\
	if (tx_status == _HTM_TBEGIN_STARTED) { \
		if(IS_LOCKED((&(lock->writer_lock)))){ \
			__TM_named_abort(255); \
		} \
		break;\
	} \
	if(__TM_is_conflict(&TM_buff)){ \
		statistics_array[local_thread_id].conflict_aborts++; \
		if(__TM_is_self_conflict(&TM_buff)) {statistics_array[local_thread_id].self_conflicts++; }\
		else if(__TM_is_trans_conflict(&TM_buff)) statistics_array[local_thread_id].trans_conflicts++; \
		else if(__TM_is_nontrans_conflict(&TM_buff)) statistics_array[local_thread_id].nontrans_conflicts++; \
		tries--; \
	} \
	else if (__TM_is_named_user_abort(&TM_buff, &code)){\
		if(code < 160){\
			statistics_array[local_thread_id].readers_aborts ++; \
			WRITER_WAIT_FUNCTION(lock);\
		}\
		else statistics_array[local_thread_id].explicit_aborts ++;\
	}\
	else if(__TM_is_footprint_exceeded(&TM_buff)){ \
		statistics_array[local_thread_id].capacity_aborts ++; \
		SPEND_BUDGET(&tries);\
		if(__TM_is_failure_persistent(&TM_buff)) statistics_array[local_thread_id].persistent_aborts ++; \
	} \
	else{ \
		statistics_array[local_thread_id].other_aborts ++; \
	} \
	tries--;\
}
#define END_HTM(){\
	__TM_end(); \
}
#define ABORT_HTM(code){\
	__TM_named_abort(code); \
}

#define TM_RESTART()                  __TM_abort();

#else /*intel*/

#define READ_WRITE_BARRIER()

#define USE_HTM(lock){\
	unsigned int status = _xbegin();\
	if (status == _XBEGIN_STARTED) {\
		if (IS_LOCKED((&(lock->writer_lock)))) {\
			ABORT_HTM(0xc8);\
		}\
		break;\
	}\
	else if (status & _XABORT_CAPACITY) {\
		SPEND_BUDGET(&tries);\
		statistics_array[local_thread_id].capacity_aborts++;\
	}\
	else {\
		if (status & _XABORT_CONFLICT) {\
			statistics_array[local_thread_id].conflict_aborts++;\
		}\
		else if (status & _XABORT_EXPLICIT) {\
			if(_XABORT_CODE(status) <= 160){ \
				statistics_array[SPECIAL_THREAD_ID()].readers_aborts++;\
				WRITER_WAIT_FUNCTION(lock);\
			}else \
			statistics_array[SPECIAL_THREAD_ID()].explicit_aborts++;\
		}\
		else if (status & _XABORT_RETRY) {\
			statistics_array[local_thread_id].retry_aborts++;\
		}\
		else if (status & _XABORT_NESTED) {\
			statistics_array[local_thread_id].nested_aborts++;\
		}\
		else {\
			statistics_array[local_thread_id].other_aborts++;\
		}\
		tries--;\
	}\
}

#define END_HTM(){\
	_xend(); \
}

#define ABORT_HTM(code){\
	/*0-159: index of reader
160: reader active
200: lock abort
255: application restart*/ \
	_xabort(code);\
}

#define TM_RESTART()                  ABORT_HTM(0xff);	//called by the application
#endif

#define TM_BEGIN_RO()                 TM_BEGIN(0)
#define TM_EARLY_RELEASE(var)

#define FAST_PATH_RESTART() TM_RESTART();
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
