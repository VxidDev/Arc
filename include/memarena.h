#ifndef ARC_ARENA_H
#define ARC_ARENA_H

#include <stddef.h>
#include <stdint.h>

typedef struct ArenaBlock {
  struct ArenaBlock* next;
  size_t size;
  size_t capacity;
  unsigned char data[];
} ArenaBlock;

typedef struct Arena {
  ArenaBlock* head;
  ArenaBlock* current;
} Arena;

extern Arena* parseArena;
extern Arena* stringArena;

Arena* arenaCreate(void);

void* arenaAlloc(Arena* arena, size_t size);
void* arenaRealloc(Arena* arena, void* old, size_t oldSize, size_t newSize);

void arenaReset(Arena* arena);   // frees everything
void arenaDestroy(Arena* arena);  // same as reset + free arena struct

#define arenaNew(arena, type) ((type*)arenaAlloc((arena), sizeof(type)))

#endif // ARC_ARENA_H
