/*
 * This file contains the structure and implimentation
 * for a thread pool in C
 * 
 * There are a number of functions defined and implimented 
 * that allow the user to use the thread pool easily
 *
 * NOTES;
 * THERE IS NO MUTEX OR CONDITION VARIABLES INCLUDED
 * ALL OTHER PTHREAD IMPLIMENTATIONS MUST BE DONE OUTSIDE
 *
 * USE THE -pthread FLAG WHEN INCLUDING THIS HEADER FILE
 *
 * Alex Austin - 6/21/2023
 */

#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct ThreadPool {
  uint32_t NumThreads;                // The number of desired threads in the pool
  pthread_t* Threads;                 // The array of threads in the pool

  void* (*ThreadAction) (void* args); // Action function the threads should do
} tpool_t;

/// PROTOTYPES ///
tpool_t* ThreadPoolInit(uint32_t num_threads, void*(*thread_action)(void*)); // Initializes the thread pool
void ThreadPoolStart(tpool_t* pool, void* thread_args);                      // Begins running the threads
void ThreadPoolJoin(tpool_t *pool);                                          // Joins all the threads together and frees them

/// IMPLIMENTATIONS ///
tpool_t* 
ThreadPoolInit(uint32_t num_threads, void* (*thread_action)(void*)) {
  tpool_t* thread_pool;

  thread_pool = (tpool_t*)malloc(sizeof(tpool_t));

  thread_pool->NumThreads = num_threads;
  thread_pool->Threads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
  thread_pool->ThreadAction = thread_action;

  return thread_pool;
}

// Starts up all of the threads in the thread pool
void
ThreadPoolStart(tpool_t *pool, void* thread_args) {
  int i;

  for(i = 0; i < pool->NumThreads; i++) {
    pthread_create(&(pool->Threads[i]), NULL, pool->ThreadAction, thread_args);
  }
}

// Joins all threads back together
// and frees them
void
ThreadPoolJoin(tpool_t* pool) {
  int i;

  for(i = 0; i < pool->NumThreads; i++) {
    pthread_join(pool->Threads[i], NULL);
  }

  free(pool->Threads);
}
