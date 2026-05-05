#include "../include/symbol-table.h"
#include "../include/utils.h"
#include "../include/object.h"

#include <stdlib.h>
#include <string.h>

unsigned long hash(const char *str) {
  unsigned long h = 5381;
  int c;

  while ((c = *str++))
    h = ((h << 5) + h) + c;

  return h;
}

SymbolTable *createTable(size_t capacity, SymbolTable *parent) {
  SymbolTable *table = malloc(sizeof(SymbolTable));
  if (!table) return NULL;

  table->capacity = capacity;
  table->parent = parent;
  table->buckets = calloc(capacity, sizeof(Symbol*));

  if (!table->buckets) {
    free(table);
    return NULL;
  }

  return table;
}

void setTable(SymbolTable *table, const char *name, Object *value) {
  unsigned long index = hash(name) % table->capacity;

  Symbol *sym = table->buckets[index];

  while (sym) {
    if (strcmp(sym->name, name) == 0) {
      freeObject(sym->value);
      sym->value = copyObject(value);
      return;
    }

    sym = sym->next;
  }

  Symbol *newSym = malloc(sizeof(Symbol));
  if (!newSym) return;

  newSym->name = stringDup(name);

  if (!newSym->name) {
    free(newSym);
    return;
  }
  
  Object *copy = copyObject(value);
  newSym->value = copy;

  newSym->next = table->buckets[index];
  table->buckets[index] = newSym;
}

void *getTable(SymbolTable *table, const char *name) {
  unsigned long index = hash(name) % table->capacity;

  Symbol *sym = table->buckets[index];

  while (sym) {
    if (strcmp(sym->name, name) == 0)
      return sym->value;
    sym = sym->next;
  }

  if (table->parent)
    return getTable(table->parent, name);

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

      free(curr->name);
      free(curr);
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

      free(sym->name);  
      freeObject(sym->value);
      free(sym);      

      sym = next;
    }
  }

  free(table->buckets);
  free(table);
}
