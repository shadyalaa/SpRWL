#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
#define CACHE_LINE_SIZE 128
#define NR_CPUS 80
#else
#define CACHE_LINE_SIZE 64
#define NR_CPUS 56
#endif

#define NR_TYPES 44

#define RHTM_THRESHOLD 0.9
struct snzi_node {
        int is_root;

        union {
                struct {
                        struct snzi_node *parent;
                        int var;
                } leaf;
                struct {
                        int state;
                        int var;
                } root;
        } x;

        char pad[CACHE_LINE_SIZE];
};

struct snzi {
        struct snzi_node root;
        struct snzi_node nodes[NR_CPUS];
};

typedef struct padded_scalar_lock {
    volatile unsigned long counter;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_l;

typedef struct padded_scalar_long {
    volatile unsigned long long counter;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_ll;

typedef struct padded_float {
    volatile float counter;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_float_t;

typedef struct padded_scalar_tracker {
        volatile unsigned int is_reader;
        volatile unsigned long clock;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t;

typedef struct padded_statistics {
        unsigned long write_commits;
        unsigned long lock_commits;
        unsigned long htm_commits;
        unsigned long read_commits;
        unsigned long conflict_aborts;
        unsigned long capacity_aborts;
        unsigned long explicit_aborts;
        unsigned long readers_aborts;
        unsigned long nontrans_conflicts;
				unsigned long trans_conflicts;
        unsigned long retry_aborts;
        unsigned long nested_aborts;
        unsigned long other_aborts;
        unsigned long self_conflicts;
        unsigned long persistent_aborts;
        unsigned long long r_avg_time;
        unsigned long long w_avg_time;
        unsigned long long start;
        unsigned long long stop;
        unsigned long long latency;
        unsigned long long r_latency;
        unsigned long long w_latency;
        unsigned long w_threads;
				unsigned long rot_commits;
				unsigned long rot_conflict_aborts;
        unsigned long rot_capacity_aborts;
        unsigned long rot_explicit_aborts;
        unsigned long rot_readers_conflicts;
        unsigned long rot_nontrans_conflicts;
				unsigned long rot_trans_conflicts;
        unsigned long rot_other_aborts;
        unsigned long rot_self_conflicts;
        unsigned long rot_persistent_aborts;
	unsigned long uro_commits;
        char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t;
#endif
