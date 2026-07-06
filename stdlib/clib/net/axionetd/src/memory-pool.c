#include "../include/memory-pool.h"

#include <stdlib.h>

axio_MemoryPool* axio_poolCreate(size_t objectSize, size_t capacity) {
    axio_MemoryPool *pool = malloc(sizeof(axio_MemoryPool));
    pool->objectSize = objectSize < sizeof(PoolNode) ? sizeof(PoolNode) : objectSize;
    pool->capacity = capacity;

    pool->memory = malloc(pool->objectSize * capacity);
    pool->freeList = NULL;

    pthread_mutex_init(&pool->lock, NULL);

    // Build free list
    for (size_t i = 0; i < capacity; i++) {
        PoolNode *node = (PoolNode*)((char*)pool->memory + i * pool->objectSize);
        node->next = pool->freeList;
        pool->freeList = node;
    }

    return pool;
}

void* axio_poolAlloc(axio_MemoryPool *pool) {
    pthread_mutex_lock(&pool->lock);

    if (!pool->freeList) {
        pthread_mutex_unlock(&pool->lock);
        return NULL; // pool exhausted
    }

    PoolNode *node = pool->freeList;
    pool->freeList = node->next;

    pthread_mutex_unlock(&pool->lock);

    return node;
}

void axio_poolFree(axio_MemoryPool *pool, void *ptr) {
    pthread_mutex_lock(&pool->lock);

    PoolNode *node = (PoolNode*)ptr;
    node->next = pool->freeList;
    pool->freeList = node;

    pthread_mutex_unlock(&pool->lock);
}

void axio_poolDestroy(axio_MemoryPool *pool) {
    if (!pool) return;

    pthread_mutex_destroy(&pool->lock);

    free(pool->memory);
    free(pool);
}
