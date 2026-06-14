#ifndef ARC_COMPILER_H
#define ARC_COMPILER_H 

#include "object.h"

typedef enum OpCode {
  OP_LOAD_CONST, // push a constant onto the stack
  OP_LOAD_VAR, // push variable value
  OP_LOAD_LOCAL,
  OP_STORE_VAR, // pop and assign to variable
  OP_STORE_LOCAL,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_POW,
  OP_NEG, // unary minus
  OP_NOT,
  OP_EQ, OP_NE, OP_LT, OP_GT, OP_LTE, OP_GTE,
  OP_AND, OP_OR,
  OP_JUMP, // unconditional jump
  OP_JUMP_IF_FALSE, // for if/while conditions
  OP_CALL, // function call
  OP_RETURN,
  OP_BREAK,
  OP_CONTINUE,
  OP_INDEX_GET,
  OP_INDEX_SET,
  OP_BUILD_LIST, // pop N items, build a list
  OP_POP,
  OP_FOR_PREP,
  OP_FOR_ITER,
  OP_TRY_PUSH,
  OP_TRY_POP,
  OP_IMPORT,
  OP_BUILD_INSTANCE,
  OP_PROPERTY_ACCESS,
  OP_PROPERTY_SET,
  OP_HALT
} OpCode;

#define MAX_LOCALS 256 // TODO: add flag for this 

typedef struct Local {
  const char *name;
  size_t len;
  int slot;
} Local;

typedef struct PosEntry {
  uint32_t offset; // first bytecode offset where this span applies
  Position start;
  Position end;
} PosEntry;

typedef struct Chunk {
  uint8_t *code;
  size_t count;
  size_t capacity;
  Object **constants;  
  size_t constCount;
  size_t constCapacity;

  PosEntry *positions;
  size_t posCount, posCapacity;

  char* filename;
  char* sourcetext;

  int maxLocals;
} Chunk;

typedef struct JumpList {
  int offset;
  struct JumpList *next;
} JumpList;

typedef struct LoopInfo {
  int start;
  JumpList *breaks;
  JumpList *continues;
  struct LoopInfo *next;
} LoopInfo;

#define INTERN_TABLE_INIT_CAP 64 // TODO: add flag for this 

typedef struct InternEntry {
  const char *str;  
  size_t len;
  uint8_t constIdx;
  bool used;
} InternEntry;

typedef struct InternTable {
  InternEntry *entries;
  size_t cap;     
  size_t count; 
} InternTable;

typedef struct Compiler {
  Chunk *chunk;
  Error **err;
  char *filename;
  char *sourcetext;
  LoopInfo *loop;

  Local locals[MAX_LOCALS];
  int localCount;
  bool isFunction; // false = top-level, no locals
  InternTable intern;

  int maxLocalCount;  

  Position posStart;
  Position posEnd;
  bool posDirty;

  const char* funcName;
  Object* funcObj;
} Compiler;

Chunk* initChunk(void);
void chunkWrite(Chunk *c, uint8_t byte);
int chunkAddConst(Chunk *c, Object *obj); // returns index
void freeChunk(Chunk* chunk);

Compiler *initCompiler(Error **err, char *filename, char *sourcetext);
Chunk *compileAST(ASTNode *ast, Error **err, char *filename, char *sourcetext);
void disassembleChunk(Chunk* chunk, const char *name);

#endif // ARC_COMPILER_H
