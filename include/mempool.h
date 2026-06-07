#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stddef.h>

typedef struct {
  void** slots;
  void* slab;
  size_t top;
  size_t objSize;
} MemPool;

extern MemPool* numberPool;
extern MemPool* stringPool;
extern MemPool* nativeFuncPool;
extern MemPool* functionCallPool;
extern MemPool* symbolPool;
extern MemPool* symbolTablePool;
extern MemPool* functionPool;

MemPool* initPool(size_t objSize); 

void* poolAlloc(MemPool* pool);
void poolFree(MemPool* pool, void* obj);

void freePool(MemPool* pool);

#endif // MEMPOOL_H
