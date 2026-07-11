#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "object.h"
#include "value.h"

typedef struct Symbol {
  char *name;
  Value value;
  struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
  Symbol **buckets;
  size_t capacity;
  size_t count;
  struct SymbolTable *parent;
} SymbolTable;

SymbolTable *createTable(size_t capacity, SymbolTable *parent);
void setTable(SymbolTable *table, char *name, Value value);
Value getTable(SymbolTable *table, const char *name);
void removeSymbol(SymbolTable *table, const char *name);
void freeTable(SymbolTable *table);

void setTableLocal(SymbolTable *table, char *name, Value value);
Value getTableLocal(SymbolTable *table, const char *name);

#endif // SYMBOL_TABLE_H
