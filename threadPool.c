//
// Created by davidregev on 03/05/19.
//

#include <malloc.h>
#include "threadPool.h"

ThreadPool* tpCreate(int numOfThreads) {
    ThreadPool* tp = (ThreadPool*)malloc(sizeof(ThreadPool));
    tp->x = numOfThreads;
    tp->tasksQueue = osCreateQueue();
    tp->threads = (pthread_t*)malloc(sizeof(pthread_t*) * numOfThreads);
    tp->is_ocuppied = (int*)calloc(sizeof(int), numOfThreads);
    tp->is_destroyed = false;
    return tp;
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {

}

int emptyCell(ThreadPool* threadPool) {
    int i;
    for (i = 0; i < threadPool->x; ++i) {
        if (threadPool->is_ocuppied[i] == 0) {
            return i;
        }
    }
    return -1;
}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    ThreadTask* tk = (ThreadTask*)malloc(sizeof(ThreadTask));
    int openCell, new_thread;
    tk->computeTask = computeFunc;
    tk->param = param;
    osEnqueue(threadPool->tasksQueue, tk);

    openCell = emptyCell(threadPool);
    if (openCell != -1) {
        //TODO create thread and run the task in it
        new_thread = pthread_create(&threadPool->threads[openCell], NULL, computeFunc, param);
    } else {
        return -1;
    }
}
