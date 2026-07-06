#ifndef REPL_H
#define REPL_H

#include <stdbool.h>

#include "../symbol-table.h"
#include "../mempool.h"
#include "../vm.h"

#define ARC_LIB_DIR "/usr/local/share/arc/lib" // TODO: add windows support 

extern char *_CODE;
extern bool _DEBUG;
extern int _FLOAT_PRECISION;
extern bool _IS_COLORED;
extern int POOL_SIZE;
extern size_t ARENA_BLOCK_SIZE;

extern VM* vm;
extern SymbolTable* variables;

#define COLOR(c) (_IS_COLORED ? (c) : "")

void initArenas();
void freeArenas();

void initMemPools();
void freeMemPools();

void registerBuiltins(SymbolTable *table);

#endif // REPL_H 
