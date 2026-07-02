#ifndef AXIO_MEMORY_POOL_H
#define AXIO_MEMORY_POOL_H

typedef struct PoolNode {
    struct PoolNode *next;
} PoolNode;

#include <pthread.h>

typedef struct {
    void *memory;
    PoolNode *freeList;
    unsigned long objectSize;
    unsigned long capacity;

    pthread_mutex_t lock;
} MemoryPool;

MemoryPool* poolCreate(size_t objectSize, size_t capacity);
void* poolAlloc(MemoryPool *pool);
void poolFree(MemoryPool *pool, void *ptr);
void poolDestroy(MemoryPool *pool);

#endif // AXIO_MEMORY_POOL_H