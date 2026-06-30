#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stddef.h>

#if defined(__GNUC__) || defined(__clang__)
  #define MP_ALIGN(n) __attribute__((aligned(n)))
  #define MP_HOT __attribute__((hot))
  #define MP_LIKELY(x) __builtin_expect(!!(x), 1)
  #define MP_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
  #define MP_ALIGN(n)
  #define MP_HOT
  #define MP_LIKELY(x) (x)
  #define MP_UNLIKELY(x) (x)
#endif

typedef struct MP_ALIGN(64) {
  void** slots;
  void* slab;
  size_t top;
  size_t cap;
  size_t objSize;
  size_t slabCount;
} MemPool;

extern MemPool* numberPool;
extern MemPool* stringPool;
extern MemPool* nativeFuncPool;
extern MemPool* functionCallPool;
extern MemPool* symbolPool;
extern MemPool* symbolTablePool;
extern MemPool* functionPool;
extern MemPool* instancePool;

MemPool* initPool(size_t objSize);
void* poolAlloc(MemPool* pool) MP_HOT;
void poolFree(MemPool* pool, void* obj) MP_HOT;
void freePool(MemPool* pool);

#endif // MEMPOOL_H
