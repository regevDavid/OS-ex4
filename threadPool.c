//
// Created by davidregev on 03/05/19.
//

#include <malloc.h>
#include <unistd.h>
#include "threadPool.h"


pthread_t* initPthread(ThreadPool* tp, int numOfThreads);
void *funcToRun(void* arg);


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
        pthread_create(&threads[i], NULL, funcToRun, tp);
    }
    return threads;
}

void freeTasks(ThreadPool* tp) {
    ThreadTask* task;
    while ((task = osDequeue(tp->tasksQueue)) != NULL) {
        free(task);
    }
}

void freeSync(ThreadPool* tp) {
    pthread_cond_destroy(&tp->sync->cond);
    pthread_mutex_destroy(&tp->sync->mutex);
}

/*
 * The function that every thread will run
 * */
void *funcToRun(void* arg) {
    ThreadPool* tp = (ThreadPool*)arg;
    // lock the functionality of the thread
    while (tp->canRun) {
        pthread_mutex_lock(&tp->sync->mutex);
        // another check because it might be the occasion where canRun
        // changed to false while the thread didn't have CPU time
        if (!tp->canRun) {
            pthread_mutex_unlock(&tp->sync->mutex);
            break;
        }
        // check if the queue is empty, if it is - put the thread on wait.
        if (osIsQueueEmpty(tp->tasksQueue) && !tp->canInsert) {
            pthread_cond_wait(&tp->sync->cond, &tp->sync->mutex);
        }else{
            ThreadTask *task;
            task = osDequeue(tp->tasksQueue);
            pthread_mutex_unlock(&tp->sync->mutex);
            task->computeTask(task->params);
            free(task);

        }

        // if there is a task in the queue - dequeue and run it

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
    tp->canInsert = true;
    tp->canRun = true;
    tp->threads = initPthread(tp, numOfThreads);
    return tp;
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    threadPool->canInsert = false;
    if (shouldWaitForTasks == 0) {
        threadPool->canRun = false;
        freeTasks(threadPool);
    }
    pthread_cond_broadcast(&threadPool->sync->cond);
    for (int i = 0; i < threadPool->x; ++i) {
        pthread_join(threadPool->threads[i], NULL);
    }

    // free resources
    freeSync(threadPool);
    free(threadPool->sync);
    free(threadPool->threads);
    osDestroyQueue(threadPool->tasksQueue);
    free(threadPool);
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    if (!threadPool->canInsert) {
        return 0;
    }
    pthread_mutex_lock(&threadPool->sync->mutex);
    ThreadTask* tk = (ThreadTask*)malloc(sizeof(ThreadTask));
    tk->computeTask = computeFunc;
    tk->params = param;
    osEnqueue(threadPool->tasksQueue, tk);
    printf("I'm signal");
//    sleep(1);
    pthread_cond_signal(&threadPool->sync->cond);
    pthread_mutex_unlock(&threadPool->sync->mutex);
    return 1;
}
