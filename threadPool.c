//
// Created by davidregev on 03/05/19.
//

#include <malloc.h>
#include <unistd.h>
#include "threadPool.h"


pthread_t* initPthread(ThreadPool* tp, int numOfThreads);
void *funcToRun(ThreadPool* tp);


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
    // lock the functionality of the thread
    while (tp->canRun) {
        pthread_mutex_lock(&tp->sync->mutex);
        // another check because it might be the occasion where canRun
        // changed to false while the thread didn't have CPU time
        if (!tp->canRun) {
            pthread_mutex_unlock(&tp->sync->mutex);
        }
        // check if the queue is empty, if it is - put the thread on wait.
        while (osIsQueueEmpty(tp->tasksQueue)) {
            pthread_cond_wait(&tp->sync->cond, &tp->sync->mutex);
        }

        // if there is a task in the queue - dequeue and run it
        ThreadTask *task;
        task = osDequeue(tp->tasksQueue);
        task->computeTask(task->params);
        free(task);
        pthread_mutex_unlock(&tp->sync->mutex);
    }
    // TODO what to return.
}

ThreadPool* tpCreate(int numOfThreads) {
    ThreadPool* tp = (ThreadPool*)malloc(sizeof(ThreadPool));

    // negative number of threads
    if (numOfThreads < 0) {
        write(2 ,"Error: Illegal number of threads.\n", 34);
    }

    tp->x = numOfThreads;

    // initializations of the structs
    tp->tasksQueue = osCreateQueue();
    tp->sync = initSynchronize();
    tp->threads = initPthread(tp, numOfThreads);
    tp->canInsert = true;
    tp->canRun = true;
    return tp;
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    threadPool->canInsert = false;
    if (shouldWaitForTasks == 0) {
        threadPool->canRun = 0;
    }
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    if (threadPool->canInsert) {
        return 0;
    }
    pthread_mutex_lock(&threadPool->sync->mutex);
    ThreadTask* tk = (ThreadTask*)malloc(sizeof(ThreadTask));
    tk->computeTask = computeFunc;
    tk->params = param;
    osEnqueue(threadPool->tasksQueue, tk);
    pthread_cond_signal(&threadPool->sync->cond);
    pthread_mutex_unlock(&threadPool->sync->mutex);
    return 1;
}
