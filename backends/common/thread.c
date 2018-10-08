/* =============================================================================
 *
 * thread.c
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * For the license of bayes/sort.h and bayes/sort.c, please see the header
 * of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of kmeans, please see kmeans/LICENSE.kmeans
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of ssca2, please see ssca2/COPYRIGHT
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/mt19937ar.c and lib/mt19937ar.h, please see the
 * header of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/rbtree.h and lib/rbtree.c, please see
 * lib/LEGALNOTICE.rbtree and lib/LICENSE.rbtree
 * 
 * ------------------------------------------------------------------------
 * 
 * Unless otherwise noted, the following license applies to STAMP files:
 * 
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 * 
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 */


#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef THREAD_C
#define THREAD_C

#include "thread.h"

__attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t statistics_array[NR_CPUS];

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_ll clocks[NR_CPUS];

__thread int local_thread_id;

__thread unsigned int ro;

__thread unsigned int t_type;

__thread unsigned int local_exec_mode = 0;

__thread unsigned int local_num_threads = 0;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_ll w_avg_cycles;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_ll r_avg_cycles;

long alpha = 1;

spec_rwlock_t* rwlock;

#ifndef REDUCED_TM_API

#include <assert.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include "types.h"
#include "random.h"
/* TODO: removed #include "rapl.h"*/

static THREAD_LOCAL_T    global_threadId;
static THREAD_BARRIER_T* global_barrierPtr      = NULL;
static long*             global_threadIds       = NULL;
static THREAD_ATTR_T     global_threadAttr;
static THREAD_T*         global_threads         = NULL;
static void            (*global_funcPtr)(void*) = NULL;
static void*             global_argPtr          = NULL;
static volatile bool_t   global_doShutdown	= FALSE;

static void
threadWait (void* argPtr)
{
    long threadId = *(long*)argPtr;

    local_num_threads = global_numThread;
	
    THREAD_LOCAL_SET(global_threadId, (long)threadId);

    bindThread(threadId);

    local_thread_id = threadId;

    while (1) {
        THREAD_BARRIER(global_barrierPtr, threadId); /* wait for start parallel */
        if (global_doShutdown) {
            break;
        }
        global_funcPtr(global_argPtr);
        THREAD_BARRIER(global_barrierPtr, threadId); /* wait for end parallel */
        if (threadId == 0) {
            break;
        }
    }
}

/* =============================================================================
 * thread_startup
 * -- Create pool of secondary threads
 * -- numThread is total number of threads (primary + secondaries)
 * =============================================================================
 */
void
thread_startup (long numThread)
{
    int i;

    global_numThread = numThread;
   
    global_doShutdown = FALSE;

    /* Set up barrier */
    assert(global_barrierPtr == NULL);
    global_barrierPtr = THREAD_BARRIER_ALLOC(numThread);
    assert(global_barrierPtr);
    THREAD_BARRIER_INIT(global_barrierPtr, numThread);

    /* Set up ids */
    THREAD_LOCAL_INIT(global_threadId);
    assert(global_threadIds == NULL);
    global_threadIds = (long*)malloc(numThread * sizeof(long));
    assert(global_threadIds);
    for (i = 0; i < numThread; i++) {
        global_threadIds[i] = i;
    }

    /* Set up thread list */
    assert(global_threads == NULL);
    global_threads = (THREAD_T*)malloc(numThread * sizeof(THREAD_T));
    assert(global_threads);
    
    /* Set up pool */
    THREAD_ATTR_INIT(global_threadAttr);
    for (i = 1; i < numThread; i++) {
        THREAD_CREATE(global_threads[i],
                      global_threadAttr,
                      &threadWait,
                      &global_threadIds[i]);
    }
}

void
thread_start (void (*funcPtr)(void*), void* argPtr)
{
    global_funcPtr = funcPtr;
    global_argPtr = argPtr;

    long threadId = 0; /* primary */
    threadWait((void*)&threadId);
}


void
thread_shutdown ()
{
    global_doShutdown= TRUE;
    /* Make secondary threads exit wait() */
    THREAD_BARRIER(global_barrierPtr, 0);

    long numThread = global_numThread;

    long i;
    for (i = 1; i < numThread; i++) {
        THREAD_JOIN(global_threads[i]);
    }

    THREAD_BARRIER_FREE(global_barrierPtr);
    global_barrierPtr = NULL;

    free(global_threadIds);
    global_threadIds = NULL;

    free(global_threads);
    global_threads = NULL;

    global_numThread = 1;
}

barrier_t *barrier_alloc() {
    return (barrier_t *)malloc(sizeof(barrier_t));
}

void barrier_free(barrier_t *b) {
    free(b);
}

void barrier_init(barrier_t *b, int n) {
    pthread_cond_init(&b->complete, NULL);
    pthread_mutex_init(&b->mutex, NULL);
    b->count = n;
    b->crossing = 0;
}

void barrier_cross(barrier_t *b) {
    pthread_mutex_lock(&b->mutex);
    /* One more thread through */
    b->crossing++;
    /* If not all here, wait */
    if (b->crossing < b->count) {
        pthread_cond_wait(&b->complete, &b->mutex);
    } else {
        /* Reset for next time */
        b->crossing = 0;
        pthread_cond_broadcast(&b->complete);
    }
    pthread_mutex_unlock(&b->mutex);
}

void
thread_barrier_wait()
{
    long threadId = thread_getId();
    THREAD_BARRIER(global_barrierPtr, threadId);
}

long
thread_getId()
{
    return (long)THREAD_LOCAL_GET(global_threadId);
}

long
thread_getNumThread()
{
    return global_numThread;
}

#endif

void snzi_init(struct snzi *obj) {
        int i = 0;

        root_init(&obj->root);
        for (i = 0; i < NR_CPUS; i++) leaf_init(&obj->nodes[i], &obj->root);
}

void snzi_inc(struct snzi *obj, unsigned int tid) {
        struct snzi_node *node = &obj->nodes[tid];
        node_arrive(node);
}

void snzi_dec(struct snzi *obj, unsigned int tid) {
        struct snzi_node *node = &obj->nodes[tid];
        node_depart(node);
}

int snzi_query(struct snzi *obj) { return node_query(&obj->root); }

void root_init(struct snzi_node *root) {
        root->is_root = 1;
        root->x.root.var = 0;
        root->x.root.state = 0;
}

void root_arrive(struct snzi_node *node) {
	int temp, x = 0;
	short c, v;
	int a;

	do {
		x = node->x.root.var;
		root_decode(x, &c, &a, &v);

		if (c == 0)
			temp = root_encode(1, 1, (short)(v + 1));
		else
			temp = root_encode((short)(c + 1), a, v);

	} while (__sync_val_compare_and_swap(&node->x.root.var, x, temp) != x);

	root_decode(temp, &c, &a, &v);

	if (a) {
		while (1) {
			int i = node->x.root.state;
			int newi = (i & 0x7FFFFFFF) + 1;
			newi = (int)(((unsigned int)newi) | 0x80000000);

			if (__sync_val_compare_and_swap(&node->x.root.state, i, newi) == i) break;
		}

		__sync_val_compare_and_swap(&node->x.root.var, root_encode(c, 0, v), temp);
	}
}

void root_depart(struct snzi_node *node) {
	while (1) {
		int x = node->x.root.var;
		short c, v;
		int a;

		root_decode(x, &c, &a, &v);

		if (__sync_val_compare_and_swap(&node->x.root.var, x,
			    root_encode((short)(c - 1), 0, v)) == x) {
			if (c >= 2) return;

			while (1) {
				int i = node->x.root.state;
				int newi;
				if (((short)(node->x.root.var >> 16)) != v)
					return;

				newi = (i & 0x7FFFFFFF) + 1;

				if (__sync_val_compare_and_swap(&node->x.root.state, i, newi) == i)
					return;
			}
		}
	}
}

int root_query(struct snzi_node *node) {
	return (node->x.root.state & 0x80000000) > 0;
}

void leaf_arrive(struct snzi_node *node) {
	int succ = 0;
	int undoArr = 0;
	int i;

	while (!succ) {
		int x = node->x.leaf.var;
		short c, v;

		leaf_decode(x, &c, &v);

		if (c >= 1) {
			if (__sync_val_compare_and_swap(&node->x.leaf.var, x,
				    leaf_encode((short)(c + 1), v)) == x)
				break;
		}

		if (c == 0) {
			int temp = leaf_encode(-1, (short)(v + 1));
			if (__sync_val_compare_and_swap(&node->x.leaf.var, x, temp) == x) {
				succ = 1;
				c = -1;
				v += 1;
				x = temp;
			}
		}

		if (c == -1) {
			node_arrive(node->x.leaf.parent);
			if (__sync_val_compare_and_swap(&node->x.leaf.var, x, leaf_encode(1, v)) !=
			    x)
				undoArr++;
		}
	}

	for (i = 0; i < undoArr; i++) node_depart(node->x.leaf.parent);
}

void leaf_depart(struct snzi_node *node) {
	while (1) {
		int x = node->x.leaf.var;
		short c, v;

		leaf_decode(x, &c, &v);

		if (__sync_val_compare_and_swap(&node->x.leaf.var, x,
			    leaf_encode((short)(c - 1), v)) == x) {
			if (c == 1) node_depart(node->x.leaf.parent);
			return;
		}
	}
}

int leaf_query(struct snzi_node *node) {
	return node_query(node->x.leaf.parent);
}

void node_arrive(struct snzi_node *node) {
	if (node->is_root)
		root_arrive(node);
	else
		leaf_arrive(node);
}

void node_depart(struct snzi_node *node) {
	if (node->is_root)
		root_depart(node);
	else
		leaf_depart(node);
}

int node_query(struct snzi_node *node) {
	if (node->is_root) return root_query(node);
	return leaf_query(node);
}


void leaf_init(struct snzi_node *leaf, struct snzi_node *parent) {
	leaf->is_root = 0;
	leaf->x.leaf.parent = parent;
	leaf->x.leaf.var = 0;
}

#endif
