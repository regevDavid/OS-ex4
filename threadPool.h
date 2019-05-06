#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <pthread.h>
#include <stdbool.h>
#include "osqueue.h"

typedef struct synchronize {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} Synchronize;

typedef struct thread_pool
{
 //The field x is here because a struct without fields
 //doesn't compile. Remove it once you add fields of your own
 int x;
 OSQueue* tasksQueue;
 pthread_t* threads;
 Synchronize* sync;
 //TODO - FILL THIS WITH YOUR FIELDS
}ThreadPool;

typedef struct thread_task
{
    void (*computeTask) (void*);
    void* params;
}ThreadTask;

Synchronize* initSynchronize();

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
