#include "../include/symbol-table.h"
#include "../include/utils.h"
#include "../include/object.h"

#include "../include/mempool.h"
#include "../include/memarena.h"

#include <stdlib.h>
#include <string.h>

static inline size_t hashPointer(const char *ptr) {
  size_t x = (size_t)ptr;
  x ^= x >> 33;
  x *= 0xff51afd7ed558ccdULL;
  x ^= x >> 33;
  x *= 0xc4ceb9fe1a85ec53ULL;
  x ^= x >> 33;
  return x;
}

SymbolTable *createTable(size_t capacity, SymbolTable *parent) {
  SymbolTable *table = poolAlloc(symbolTablePool);
  if (!table) return NULL; 

  table->capacity = capacity;
  table->count = 0;
  table->parent = parent;
  table->buckets = calloc(capacity, sizeof(Symbol*)); 

  if (!table->buckets) {
    return NULL;
  }

  return table;
}

void setTable(SymbolTable *table, char *name, Value value) {
  size_t hash = hashPointer(name);
  
  for (SymbolTable *currTable = table; currTable; currTable = currTable->parent) {
    if (currTable->count == 0) continue;
    size_t index = hash & (currTable->capacity - 1); // capacity differs per table
    for (Symbol *sym = currTable->buckets[index]; sym; sym = sym->next) {
      if (sym->name == name) {
        freeValue(sym->value);
        sym->value = IS_OBJ(value) ? copyValue(value) : value;
        return;
      }
    }
  }
  
  size_t index = hash & (table->capacity - 1);
  Symbol *newSym = poolAlloc(symbolPool);
  if (!newSym) return;

  newSym->name = name;

  newSym->value = IS_OBJ(value) ? copyValue(value) : value;

  newSym->next = table->buckets[index];
  table->buckets[index] = newSym;
  table->count++;
}

Value getTable(SymbolTable *table, const char *name) {
  size_t hash = hashPointer(name);

  for (SymbolTable *currTable = table; currTable; currTable = currTable->parent) {
    if (currTable->count == 0) continue;

    size_t index = hash & (currTable->capacity - 1);
    Symbol *sym = currTable->buckets[index];
    Symbol *prev = NULL;

    while (sym) {
      if (sym->name == name) {
        if (prev) {
          prev->next = sym->next;
          sym->next = currTable->buckets[index];
          currTable->buckets[index] = sym;
        }
        return sym->value;
      }

      prev = sym;
      sym = sym->next;
    }
  }

  return VAL_UNDEF();
}

/*
void removeSymbol(SymbolTable *table, const char *name) {
  if (table->count == 0) return;

  unsigned long index = hashPointer(name) & (table->capacity - 1);

  Symbol *curr = table->buckets[index];
  Symbol *prev = NULL;

  while (curr) {
    if (curr->name == name) {
      if (prev)
        prev->next = curr->next;
      else
        table->buckets[index] = curr->next;

      freeObject(curr->value);
      poolFree(symbolPool, curr);
      table->count--;

      return;
    }

    prev = curr;
    curr = curr->next;
  }
}
*/
void freeTable(SymbolTable *table) {
  if (!table) return;

  if (table->count > 0) {
    for (size_t i = 0; i < table->capacity; i++) {
      Symbol *sym = table->buckets[i];

      while (sym) {
        Symbol *next = sym->next;
        
        freeValue(sym->value);
        poolFree(symbolPool, sym);

        sym = next;
      }
    }
  }

  free(table->buckets);

  table->parent = NULL;  
  table->buckets = NULL;  
  table->capacity = 0;   
  table->count = 0;

  poolFree(symbolTablePool, table);
}
