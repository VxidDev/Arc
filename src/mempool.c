#include "../include/mempool.h"
#include "../include/repl/repl.h"
#include "../include/memarena.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#include <malloc.h>

static inline int arcAlignedAlloc(void **ptr, size_t alignment, size_t size) {
  *ptr = _aligned_malloc(size, alignment);
  return (*ptr == NULL) ? -1 : 0;
}

static inline void arcAlignedFree(void *ptr) {
  _aligned_free(ptr);
}

#else 

static inline int arcAlignedAlloc(void **ptr, size_t alignment, size_t size) {
  return posix_memalign(ptr, alignment, size);
}

static inline void arcAlignedFree(void *ptr) {
  free(ptr);
}

#endif // _WIN32

#define POOL_GROWTH_FACTOR 2

MemPool* initPool(size_t objSize) {
  if (objSize == 0) return NULL;

  MemPool* pool = NULL;
  
  if (arcAlignedAlloc((void**)&pool, 64, sizeof(MemPool)) != 0 || !pool)
    return NULL;

  pool->slab = arenaAlloc(poolArena, objSize * POOL_SIZE);
  if (!pool->slab) { free(pool); return NULL; }

  memset(pool->slab, 0, objSize * POOL_SIZE);

  pool->slots = malloc(POOL_SIZE * sizeof(void*));
  if (!pool->slots) { free(pool); return NULL; }

  char* base = (char*)pool->slab;
  
  for (size_t i = 0; i < (size_t)POOL_SIZE; i++)
    pool->slots[i] = base + i * objSize;

  pool->top = POOL_SIZE;
  pool->cap = POOL_SIZE;
  pool->objSize = objSize;
  pool->slabCount = POOL_SIZE;

  return pool;
}

static inline int _isSlabPtr(const MemPool* pool, const void* ptr) {
  const char* p = (const char*)ptr;
  const char* start = (const char*)pool->slab;
  const char* end = start + pool->objSize * pool->slabCount;
  return p >= start && p < end;
}

static int _growSlots(MemPool* pool) {
  size_t newCap = pool->cap * POOL_GROWTH_FACTOR;
  void** newSlots = realloc(pool->slots, newCap * sizeof(void*));
  if (!newSlots) return 0;
  pool->slots = newSlots;
  pool->cap = newCap;
  return 1;
}

void* poolAlloc(MemPool* pool) {
  if (MP_UNLIKELY(!pool)) return NULL;

  if (MP_LIKELY(pool->top > 0))
    return pool->slots[--pool->top];

  return malloc(pool->objSize);
}

void poolFree(MemPool* pool, void* obj) {
  if (MP_UNLIKELY(!pool || !obj)) return;

  if (MP_UNLIKELY(!_isSlabPtr(pool, obj))) {
    free(obj);
    return;
  }

  if (MP_UNLIKELY(pool->top >= pool->cap)) {
    if (!_growSlots(pool)) return;
  }

  pool->slots[pool->top++] = obj;
}

void freePool(MemPool* pool) {
  if (!pool) return;
  free(pool->slots);
  arcAlignedFree(pool);
}
