#include "../include/symbol-table.h"
#include "../include/utils.h"
#include "../include/object.h"

#include "../include/mempool.h"
#include "../include/memarena.h"

#include <stdlib.h>
#include <string.h>

unsigned long hash(const char *str) {
  unsigned long h = 1469598103934665603ULL;
  unsigned long prime = 1099511628211ULL;

  unsigned char c;

  while ((c = (unsigned char)*str++)) {
    h ^= c;
    h *= prime;
  }

  return h;
}

SymbolTable *createTable(size_t capacity, SymbolTable *parent) {
  SymbolTable *table = poolAlloc(symbolTablePool);
  if (!table) return NULL; 

  table->capacity = capacity;
  table->parent = parent;
  table->buckets = calloc(capacity, sizeof(Symbol*)); 

  if (!table->buckets) {
    return NULL;
  }

  return table;
}

void setTable(SymbolTable *table, char *name, Object *value, bool copyObj) {
  unsigned long index = hash(name) % table->capacity;

  Symbol *sym = table->buckets[index];

  while (sym) {
    if (strcmp(sym->name, name) == 0) {
      freeObject(sym->value);
      sym->value = copyObj ? copyObject(value) : value;
      return;
    }

    sym = sym->next;
  }

  Symbol *newSym = poolAlloc(symbolPool);
  if (!newSym) return;

  // newSym->name = stringDup(name);
  newSym->name = name;
  
  //if (!newSym->name) {
  //  return;
  //}
  
  Object *copy = copyObj ? copyObject(value) : value;
  newSym->value = copy;

  newSym->next = table->buckets[index];
  table->buckets[index] = newSym;
}

Object *getTable(SymbolTable *table, const char *name) {
  SymbolTable *currTable = table;
  while (currTable) {
    unsigned long index = hash(name) % currTable->capacity;
    Symbol *sym = currTable->buckets[index];

    while (sym) {
      if (strcmp(sym->name, name) == 0)
        return sym->value;

      sym = sym->next;
    }

    currTable = currTable->parent;
  }

  return NULL;
}

void removeSymbol(SymbolTable *table, const char *name) {
  unsigned long index = hash(name) % table->capacity;

  Symbol *curr = table->buckets[index];
  Symbol *prev = NULL;

  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      if (prev)
        prev->next = curr->next;
      else
        table->buckets[index] = curr->next;
      
      freeObject(curr->value);
      poolFree(symbolPool, curr);

      return;
    }

    prev = curr;
    curr = curr->next;
  }
}

void freeTable(SymbolTable *table) {
  if (!table) return;

  for (size_t i = 0; i < table->capacity; i++) {
    Symbol *sym = table->buckets[i];

    while (sym) {
      Symbol *next = sym->next;

      freeObject(sym->value);
      poolFree(symbolPool, sym);

      sym = next;
    }
  }

  free(table->buckets);

  table->parent = NULL;  
  table->buckets = NULL;  
  table->capacity = 0;   

  poolFree(symbolTablePool, table);
}
