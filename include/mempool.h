#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stddef.h>

typedef struct {
  void** slots;
  int top;
  size_t objSize;
} MemPool;

extern MemPool* numberPool;
extern MemPool* stringPool;

MemPool* initPool(size_t objSize); 

void* poolAlloc(MemPool* pool);
void poolFree(MemPool* pool, void* obj);

void freePool(MemPool* pool);

#endif // MEMPOOL_H
