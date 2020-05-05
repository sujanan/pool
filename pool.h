#ifndef POOL_H
#define POOL_H

#include <pthread.h>
#include <semaphore.h>

typedef void (*taskFn)(void *);

/// task
/// ====
/// @fn: task function.
/// @arg: Arguments of the @fn.
struct task {
    taskFn fn;
    void *arg; 
};

/// Max number of tasks, task queue can hold.
#define TASK_QUEUE_SIZE 16

/// taskQueue
/// =========
/// @tasks: Tasks array.
/// @head: Queue head position.
/// @tail: Queue tail position.
/// @n: Number of tasks in queue.
struct taskQueue {
    struct task tasks[TASK_QUEUE_SIZE];
    int head;
    int tail;
    int n;
};

/// Number of workers (threads) in the thread pool.
#define POOL_WORKER_SIZE 4

/// pool
/// ====
/// @fcount: Fill count semaphore.
/// @ecount: Empty count semaphore.
/// @lock: Locking semaphore for mutual execution.
/// @q: Task queue.
/// @workers: Working threads of the pool.
struct pool {
    sem_t fcount;
    sem_t ecount;
    sem_t lock;
    struct taskQueue q;
    pthread_t workers[POOL_WORKER_SIZE];
};

/// ========
/// Main API
/// ========
void poolInit(struct pool *p);
void poolFree(struct pool *p);
void poolCreateWorkers(struct pool *p);
void poolShutdown(struct pool *p);
void poolDestroyWorkers(struct pool *p);
void poolAddTask(struct pool *p, struct task t);

#endif

