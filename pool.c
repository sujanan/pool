#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "pool.h"

#define ARRAYLEN(a) (sizeof(a)/sizeof(a[0]))

#define taskQueueInit(q) ((q)->head = (q)->tail = (q)->n = 0)
#define taskQueueFree(q) taskQueueInit(q)
#define taskQueueIsEmpty(q) ((q)->n == 0)

void taskQueueEnqueue(struct taskQueue *q, struct task task) {
    q->tasks[q->tail] = task;
    q->tail = (q->tail + 1) % ARRAYLEN((q)->tasks);
    q->n++;
}

struct task taskQueueDequeue(struct taskQueue *q) {
    struct task task = q->tasks[q->head];
    q->head = (q->head + 1) % ARRAYLEN((q)->tasks);
    q->n--;
    return task; 
}

#define UP(s) (sem_post(s))
#define DOWN(s) (sem_wait(s))
#define FILLCOUNT_UP(p) UP(&p->fcount)
#define FILLCOUNT_DOWN(p) DOWN(&p->fcount)
#define EMPTYCOUNT_UP(p) UP(&p->ecount)
#define EMPTYCOUNT_DOWN(p) DOWN(&p->ecount)
#define LOCK(p) DOWN(&p->lock)
#define UNLOCK(p) UP(&p->lock)

static void *worker(void *arg) {
    struct pool *p = (struct pool *)arg;

    while (1) {
        FILLCOUNT_DOWN(p);
        LOCK(p);
        if (taskQueueIsEmpty(&p->q))
            break;
        struct task t = taskQueueDequeue(&p->q);
        UNLOCK(p);
        EMPTYCOUNT_UP(p);
        (*t.fn)(t.arg);
    }
    UNLOCK(p);
}

void poolInit(struct pool *p) {
    taskQueueInit(&p->q);
    sem_init(&p->fcount, 0, 0);
    sem_init(&p->ecount, 0, TASK_QUEUE_SIZE);
    sem_init(&p->lock, 0, 1);
}

void poolFree(struct pool *p) {
    sem_destroy(&p->fcount);
    sem_destroy(&p->ecount);
    sem_destroy(&p->lock);
    taskQueueFree(&p->q);
}

void poolCreateWorkers(struct pool *p) {
    for (int i = 0; i < ARRAYLEN(p->workers); i++)
        pthread_create(&p->workers[i], NULL, worker, (void *)p);
}

void poolShutdown(struct pool *p) {
    for (int i = 0; i < ARRAYLEN(p->workers); i++)
        FILLCOUNT_UP(p);
}

void poolDestroyWorkers(struct pool *p) {
    for (int i = 0; i < ARRAYLEN(p->workers); i++)
        pthread_join(p->workers[i], NULL);
}

void poolAddTask(struct pool *p, struct task t) {
    EMPTYCOUNT_DOWN(p);
    LOCK(p);
    taskQueueEnqueue(&p->q, t);
    UNLOCK(p);
    FILLCOUNT_UP(p);
}

