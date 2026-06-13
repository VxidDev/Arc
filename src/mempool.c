#include "../include/mempool.h"
#include "../include/repl/repl.h"

#include <stdlib.h>
#include <string.h>

MemPool* initPool(size_t objSize) {
  if (objSize == 0) return NULL;

  MemPool* pool = malloc(sizeof(MemPool));
  if (!pool) return NULL;

  pool->slab = malloc(objSize * POOL_SIZE);

  if (!pool->slab) {
    free(pool);
    return NULL;
  }

  memset(pool->slab, 0, objSize * POOL_SIZE);

  pool->slots = malloc(sizeof(void*) * POOL_SIZE);

  if (!pool->slots) { 
    free(pool->slab);
    free(pool);
    return NULL; 
  }


  for (int i = 0; i < POOL_SIZE; i++)
    pool->slots[i] = (char*)pool->slab + i * objSize;

  pool->top = POOL_SIZE;
  pool->objSize = objSize;

  return pool;
}

static inline int _isSlabPtr(MemPool* pool, void* ptr) {
  char* p = (char*)ptr;
  char* start = (char*)pool->slab;
  char* end = start + pool->objSize * POOL_SIZE;

  return p >= start && p < end;
}

void* poolAlloc(MemPool* pool) {
  if (!pool) return NULL;

  if (pool->top > 0)
    return pool->slots[--pool->top];

  return malloc(pool->objSize);
}

void poolFree(MemPool* pool, void* obj) {
  if (!pool || !obj) return;

  if (pool->top < (size_t)POOL_SIZE)
    pool->slots[pool->top++] = obj;
  else
    if (!_isSlabPtr(pool, obj)) free(obj);
    // slab pointers get dropped and will get freed when freePool is called.
}

void freePool(MemPool* pool) {
  if (!pool) return;

  // free any heap-overflow objects sitting in the freelist
  for (size_t i = 0; i < pool->top; i++) {
    if (!_isSlabPtr(pool, pool->slots[i]))
      free(pool->slots[i]);
  }

  free(pool->slots);
  free(pool->slab);
  free(pool);
}
