#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "object.h"

typedef struct Symbol {
  char *name;
  void *value;
  struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
  Symbol **buckets;
  unsigned long capacity;
  struct SymbolTable *parent;
} SymbolTable;

SymbolTable *createTable(unsigned long capacity, SymbolTable *parent);
void setTable(SymbolTable *table, const char *name, Number *value);
void *getTable(SymbolTable *table, const char *name);
void removeSymbol(SymbolTable *table, const char *name);
void freeTable(SymbolTable *table);

#endif // SYMBOL_TABLE_H
