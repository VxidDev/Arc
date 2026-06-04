#include "../include/mempool.h"
#include "../include/repl/repl.h"

#include <stdlib.h>

MemPool* initPool(size_t objSize) {
  if (POOL_SIZE <= 0) return NULL;

  MemPool* pool = malloc(sizeof(MemPool));
  if (!pool) return NULL;

  pool->slots = malloc(sizeof(void*) * POOL_SIZE);

  if (!pool->slots) { 
    free(pool);
    return NULL; 
  }

  pool->top = 0;
  pool->objSize = objSize;

  return pool;
}

void* poolAlloc(MemPool* pool) {
  if (!pool) return NULL;

  if (pool->top > 0)
    return pool->slots[--pool->top];

  return malloc(pool->objSize);
}

void poolFree(MemPool* pool, void* obj) {
  if (!pool || !obj) return;

  if (pool->top < POOL_SIZE)
    pool->slots[pool->top++] = obj;
  else
    free(obj);
}

void freePool(MemPool* pool) {
  if (!pool) return;

  for (int i = 0; i < pool->top; i++)
    free(pool->slots[i]);

  free(pool->slots);
  free(pool);
}
