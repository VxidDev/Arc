#include "../include/memarena.h"
#include "../include/repl/repl.h"

#include <stdlib.h>
#include <string.h>

static ArenaBlock* arenaNewBlock(size_t minSize) {
  size_t cap = minSize > ARENA_BLOCK_SIZE ? minSize : ARENA_BLOCK_SIZE;

  ArenaBlock* block = (ArenaBlock*)malloc(sizeof(ArenaBlock) + cap);
  
  if (!block) return NULL;

  block->next = NULL;
  block->size = 0;
  block->capacity = cap;

  return block;
}

Arena* arenaCreate(void) {
  Arena* arena = (Arena*)malloc(sizeof(Arena));
  if (!arena) return NULL;

  arena->head = arenaNewBlock(ARENA_BLOCK_SIZE);
  
  if (!arena->head) {
    free(arena);
    return NULL;
  }

  arena->current = arena->head;

  return arena;
}

void* arenaAlloc(Arena* arena, size_t size) {
  if (!arena || size == 0) return NULL;

  size = (size + 7) & ~(size_t)7;

  ArenaBlock* block = arena->current;

  if (!block || block->size + size > block->capacity) {
    ArenaBlock* nb = arenaNewBlock(size);

    if (!nb) return NULL;

    nb->next = arena->head;
    arena->head = arena->current = nb;
    block = nb;
  }
  void* ptr = block->data + block->size;
  block->size += size;

  return ptr;
}

void* arenaRealloc(Arena* arena, void* old, size_t oldSize, size_t newSize) {
  if (!arena || newSize == 0) return NULL;

  oldSize = (oldSize + 7) & ~(size_t)7;
  newSize = (newSize + 7) & ~(size_t)7;

  ArenaBlock* block = arena->current;

  if (block && old == (void*)(block->data + block->size - oldSize) && block->size - oldSize + newSize <= block->capacity) {
    block->size = block->size - oldSize + newSize;
    return old;
  }

  void* mem = arenaAlloc(arena, newSize);

  if (!mem) return NULL;

  if (old && oldSize > 0) 
    memcpy(mem, old, oldSize < newSize ? oldSize : newSize);

  return mem;
}

void arenaReset(Arena* arena) {
  if (!arena) return;

  ArenaBlock* b = arena->head;

  while (b) {
    ArenaBlock* next = b->next;
    free(b);
    b = next;
  }

  arena->head = NULL;
}

void arenaDestroy(Arena* arena) {
  if (!arena) return;

  arenaReset(arena);
  free(arena);
}
