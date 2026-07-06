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
} axio_MemoryPool;

axio_MemoryPool* axio_poolCreate(size_t objectSize, size_t capacity);
void* axio_poolAlloc(axio_MemoryPool *pool);
void axio_poolFree(axio_MemoryPool *pool, void *ptr);
void axio_poolDestroy(axio_MemoryPool *pool);

#endif // AXIO_MEMORY_POOL_H
