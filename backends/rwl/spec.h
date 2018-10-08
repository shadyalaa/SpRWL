#ifndef SPEC_H
#define SPEC_H 1

#include <pthread.h>


typedef pthread_rwlock_t spec_rwlock_t;

#define TM_INIT() TM_INIT_MUT(rwlock)

#define TM_INIT_MUT(lock) lock=(pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t)); pthread_rwlock_init(lock,NULL);

#define ACQUIRE_READ_LOCK(lock){\
	START_LATENCY(); \
        while((pthread_rwlock_rdlock(lock) != 0)) cpu_relax(); \
	START_TRANSACTION();\
}

#define ACQUIRE_WRITE_LOCK(lock){\
	START_LATENCY();\
        while((pthread_rwlock_wrlock(lock) != 0)) cpu_relax(); \
	START_TRANSACTION();\
}

#define RELEASE_READ_LOCK(lock){\
	pthread_rwlock_unlock(lock);\
	statistics_array[local_thread_id].lock_commits++; \
	END_RDTSC_R();\
}

#define RELEASE_WRITE_LOCK(lock){\
        pthread_rwlock_unlock(lock); \
	statistics_array[local_thread_id].lock_commits++; \
	END_RDTSC_W();\
}

#endif
