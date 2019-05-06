//
// Created by davidregev on 03/05/19.
//

#include <malloc.h>
#include <unistd.h>
#include "threadPool.h"


Synchronize* initSynchronize();
pthread_t* initPthread(ThreadPool* pt, int numOfThreads);
void *funcToRun(ThreadPool* tp);
ThreadPool* tpCreate(int numOfThreads);
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);


Synchronize* initSynchronize() {
    Synchronize* sync = (Synchronize*)malloc(sizeof(Synchronize));
    pthread_cond_init(&sync->cond, NULL);
    pthread_mutex_init(&sync->mutex, NULL);
    return sync;
}

pthread_t* initPthread(ThreadPool* tp, int numOfThreads) {
    int i;
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t*) * numOfThreads);
    for (i = 0; i < numOfThreads; ++i) {
        threads[i] = pthread_create(&threads[i], NULL, funcToRun(tp), NULL);
    }
    return threads;
}

/*
 * The function that every thread will run
 * */
void *funcToRun(ThreadPool* tp) {
    pthread_mutex_lock(&tp->sync->mutex);
    while (osIsQueueEmpty(tp->tasksQueue)) {
        pthread_cond_wait(&tp->sync->cond, &tp->sync->mutex);
    }
    ThreadTask* task = (ThreadTask*)malloc(sizeof(ThreadTask));
    task = osDequeue(tp->tasksQueue);
    task->computeTask(task->params);
    free(task);
    pthread_mutex_unlock(&tp->sync->mutex);
}

ThreadPool* tpCreate(int numOfThreads) {
    ThreadPool* tp = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (numOfThreads < 0) {
        write(2 ,"Error: Illegal number of threads.\n", 34);
    }
    tp->x = numOfThreads;
    tp->tasksQueue = osCreateQueue();
    tp->sync = initSynchronize();
    tp->threads = initPthread(tp, numOfThreads);
    return tp;
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    // TODO check if there are trheds to wait for.
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    pthread_mutex_lock(&mutex);
    ThreadTask* tk = (ThreadTask*)malloc(sizeof(ThreadTask));
    tk->computeTask = computeFunc;
    tk->params = param;
    osEnqueue(threadPool->tasksQueue, tk);
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
}
