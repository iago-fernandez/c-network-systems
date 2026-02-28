/**
 * @file thread_pool.h
 * @brief Defines the thread pool interface for concurrent task execution.
 */
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdint.h>

typedef void (*task_func_t)(void* arg);

typedef struct {
    task_func_t function;
    void* argument;
} Task;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t* threads;
    Task* queue;
    uint32_t thread_count;
    uint32_t queue_size;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    int shutdown;
} ThreadPool;

/**
 * @brief Allocates and initializes a new thread pool.
 */
ThreadPool* thread_pool_create(uint32_t thread_count, uint32_t queue_size);

/**
 * @brief Adds a new task to the thread pool queue.
 */
int thread_pool_add_task(ThreadPool* pool, task_func_t function, void* argument);

/**
 * @brief Destroys the thread pool and releases all resources.
 */
void thread_pool_destroy(ThreadPool* pool);

#endif