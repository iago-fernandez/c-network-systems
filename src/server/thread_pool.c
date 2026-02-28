/**
 * @file thread_pool.c
 * @brief Implementation of the thread pool using a circular buffer task queue.
 */
#include "server/thread_pool.h"
#include "common/logger.h"
#include <stdlib.h>
#include <unistd.h>

 // Worker thread routine to consume and execute tasks
static void* thread_pool_worker(void* thread_pool) {
    ThreadPool* pool = (ThreadPool*)thread_pool;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while (pool->count == 0 && !pool->shutdown) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->shutdown && pool->count == 0) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        Task task = pool->queue[pool->head];
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        pthread_mutex_unlock(&(pool->lock));

        (*(task.function))(task.argument);
    }

    return NULL;
}

ThreadPool* thread_pool_create(uint32_t thread_count, uint32_t queue_size) {
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (pool == NULL) {
        return NULL;
    }

    pool->thread_count = thread_count;
    pool->queue_size = queue_size;
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    pool->shutdown = 0;

    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    pool->queue = (Task*)malloc(sizeof(Task) * queue_size);

    if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
        pthread_cond_init(&(pool->notify), NULL) != 0 ||
        pool->threads == NULL || pool->queue == NULL) {
        thread_pool_destroy(pool);
        return NULL;
    }

    for (uint32_t i = 0; i < thread_count; i++) {
        if (pthread_create(&(pool->threads[i]), NULL, thread_pool_worker, (void*)pool) != 0) {
            thread_pool_destroy(pool);
            return NULL;
        }
    }

    return pool;
}

int thread_pool_add_task(ThreadPool* pool, task_func_t function, void* argument) {
    if (pool == NULL || function == NULL) {
        return -1;
    }

    pthread_mutex_lock(&(pool->lock));

    if (pool->count == pool->queue_size || pool->shutdown) {
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    pool->queue[pool->tail].function = function;
    pool->queue[pool->tail].argument = argument;
    pool->tail = (pool->tail + 1) % pool->queue_size;
    pool->count += 1;

    pthread_cond_signal(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

void thread_pool_destroy(ThreadPool* pool) {
    if (pool == NULL) {
        return;
    }

    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = 1;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    if (pool->threads != NULL) {
        for (uint32_t i = 0; i < pool->thread_count; i++) {
            pthread_join(pool->threads[i], NULL);
        }
        free(pool->threads);
    }

    if (pool->queue != NULL) {
        free(pool->queue);
    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));
    free(pool);
}