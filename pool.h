#ifndef POOL_H
#define POOL_H

#include <pthread.h>
#include <semaphore.h>

typedef void (*taskFn)(void *);

struct task {
    taskFn fn;
    void *arg; 
};

#define TASK_QUEUE_SIZE 16

struct taskQueue {
    struct task tasks[TASK_QUEUE_SIZE];
    int head;
    int tail;
    int n;
};

#define POOL_WORKER_SIZE 4

struct pool {
    sem_t fcount;
    sem_t ecount;
    sem_t lock;
    struct taskQueue q;
    pthread_t workers[POOL_WORKER_SIZE];
};

void poolInit(struct pool *p);
void poolFree(struct pool *p);
void poolCreateWorkers(struct pool *p);
void poolShutdown(struct pool *p);
void poolDestroyWorkers(struct pool *p);
void poolAddTask(struct pool *p, struct task t);

#endif

