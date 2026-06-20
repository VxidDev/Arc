#include "../include/compiler.h"
#include "../include/memarena.h"
#include "../include/node.h"
#include "../include/object.h"
#include "../include/utils.h"
#include "../include/repl/repl.h"

#include "../include/mempool.h"

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>

static void chunkAddPosEntry(Chunk *chunk, uint32_t offset, Position start, Position end) {
  if (chunk->posCount > 0 && chunk->positions[chunk->posCount - 1].offset == offset) {
    chunk->positions[chunk->posCount - 1].start = start;
    chunk->positions[chunk->posCount - 1].end = end;
    return;
  }

  if (chunk->posCount >= chunk->posCapacity) {
    size_t oldCap = chunk->posCapacity;
    size_t newCap = oldCap == 0 ? 64 : oldCap * 2;

    PosEntry *grown = arenaRealloc(objectArena, chunk->positions, oldCap * sizeof(PosEntry), newCap * sizeof(PosEntry));
    if (!grown) return;

    chunk->positions = grown;
    chunk->posCapacity = newCap;
  }

  chunk->positions[chunk->posCount++] = (PosEntry){
    .offset = offset, .start = start, .end = end
  };
}

static inline void setPos(Compiler *c, Position start, Position end) {
  c->posStart = start;
  c->posEnd = end;
  c->posDirty = true;
}

static inline void setPosFromNode(Compiler *c, ASTNode *node) {
  Position s = getNodeStart(node);
  Position e = getNodeEnd(node);
  setPos(c, s, e);
}

static bool internTableResize(InternTable *t, size_t oldCap, size_t newCap) {
  if (_DEBUG) printf("[debug] Resizing intern table.\n");

  InternEntry *newEntries = arenaRealloc(objectArena, t->entries, oldCap * sizeof(InternEntry), newCap * sizeof(InternEntry));
  if (!newEntries) return false;

  t->entries = newEntries;
  t->cap = newCap;

  return true;
}

static uint8_t internString(Compiler *c, char *str, size_t len) {
  if (!c->intern.entries) {
    size_t cap = INTERN_TABLE_INIT_CAP;
    c->intern.entries = arenaAlloc(objectArena, cap * sizeof(InternEntry));
    if (!c->intern.entries) goto oom;

    memset(c->intern.entries, 0, cap * sizeof(InternEntry));

    c->intern.cap = cap;
    c->intern.count = 0;
  }

  uint32_t hash = hashStr(str, len);
  size_t slot = hash & (c->intern.cap - 1);

  while (c->intern.entries[slot].used) {
    InternEntry *e = &c->intern.entries[slot];

    if (e->len == len && memcmp(e->str, str, len) == 0)
      return e->constIdx;  /* cache hit */

    slot = (slot + 1) & (c->intern.cap - 1);
  }

  if (c->chunk->constCount >= 256) {
    if (c->err && !*c->err)
      *c->err = initRuntimeError(
        (Position){0,0,0}, (Position){0,0,0},
        c->filename,
        "Too many constants in chunk (max 256).",
        c->sourcetext);

    return 0;
  }

  Object *obj;
  if (len > 0) {
    char* interned = internIdentifier(str, len);
    obj = (Object *)initStringConst(interned, len);
  } else {
    obj = (Object *)initString(str, len);
  }

  if (!obj) goto oom;

  int raw = chunkAddConst(c->chunk, obj);
  if (raw < 0) goto oom;

  uint8_t idx = (uint8_t)raw;

  if (c->intern.count * 4 >= c->intern.cap * 3) {
    if (!internTableResize(&c->intern, c->intern.cap, c->intern.cap * 2)) goto oom;

    /* Recompute slot in the new table. */
    slot = hash & (c->intern.cap - 1);

    while (c->intern.entries[slot].used)
      slot = (slot + 1) & (c->intern.cap - 1);
  }

  c->intern.entries[slot] = (InternEntry){
    .str = ((String *)obj)->value,   /* interned pointer */
    .len = len,
    .constIdx = idx,
    .used = true,
  };

  c->intern.count++;

  return idx;

oom:
  if (c->err && !*c->err)
    *c->err = initRuntimeError(
    (Position){0,0,0}, (Position){0,0,0},
    c->filename, "Out of memory.", c->sourcetext);
  return 0;
}

static uint8_t addStringConst(Compiler *c, char *str, size_t len) {
  if (c->chunk->constCount >= 256) {
    if (c->err && !*c->err)
      *c->err = initRuntimeError(
        (Position){0,0,0}, (Position){0,0,0},
        c->filename,
        "Too many constants in chunk (max 256).",
        c->sourcetext);

    return 0;
  }

  Object *obj = (Object *)initString(str, len);
  if (!obj) {
    if (c->err && !*c->err)
      *c->err = initRuntimeError(
      (Position){0,0,0}, (Position){0,0,0},
      c->filename, "Out of memory.", c->sourcetext);
    return 0;
  }

  int raw = chunkAddConst(c->chunk, obj);
  if (raw < 0) return 0;

  return (uint8_t)raw;
}


#define CHUNK_INIT_CAP 256 // TODO: add flag for this 
#define CONST_INIT_CAP 64 // TODO: add flag for this 

Chunk *initChunk(void) {
  Chunk *chunk = arenaNew(objectArena, Chunk);
  if (!chunk) return NULL;

  memset(chunk, 0, sizeof(Chunk));

  chunk->code = arenaAlloc(objectArena, CHUNK_INIT_CAP * sizeof(uint8_t));
  if (!chunk->code) return NULL;
  
  memset(chunk->code, 0, CHUNK_INIT_CAP * sizeof(uint8_t));

  chunk->count = 0; 
  chunk->capacity = CHUNK_INIT_CAP;

  chunk->constants = arenaAlloc(objectArena, CONST_INIT_CAP * sizeof(Object *));
  if (!chunk->constants) return NULL;

  memset(chunk->constants, 0, CONST_INIT_CAP * sizeof(Object*));

  chunk->constCount = 0; 
  chunk->constCapacity = CONST_INIT_CAP;

  chunk->positions = NULL;
  chunk->posCount = 0;
  chunk->posCapacity = 0;
  chunk->filename = NULL;
  chunk->sourcetext = NULL;

  chunk->maxLocals = 0;

  return chunk;
}

void chunkWrite(Chunk *chunk, uint8_t byte) {
  if (!chunk) return;

  if (chunk->count >= chunk->capacity) {
    size_t oldCap = chunk->capacity;
    size_t newCap = oldCap * 2;
    uint8_t *grown = arenaRealloc(objectArena, chunk->code, oldCap * sizeof(uint8_t), newCap * sizeof(uint8_t));
    if (!grown) return;

    chunk->code = grown;
    chunk->capacity = newCap;
  }

  chunk->code[chunk->count++] = byte;
}

int chunkAddConst(Chunk *chunk, Object *obj) {
  if (!chunk || !obj) return -1;
  
  obj->isStatic = true;

  if (chunk->constCount >= chunk->constCapacity) {
    size_t oldCap = chunk->constCapacity;
    size_t newCap = oldCap * 2;

    Object **grown = arenaRealloc(objectArena, chunk->constants, oldCap * sizeof(Object *), newCap * sizeof(Object *));
    if (!grown) return -1;

    chunk->constants = grown;
    chunk->constCapacity = newCap;
  }

  int idx = (int)chunk->constCount;
  chunk->constants[chunk->constCount++] = obj;

  return idx;
}

void freeChunk(Chunk *chunk) {
  if (!chunk) return;

  for (size_t i = 0; i < chunk->constCount; i++) {
    Object *obj = chunk->constants[i];
    if (!obj) continue;

    if (obj->type == OBJ_FUNCTION) {
      Function *func = (Function *)obj;
      Chunk *subChunk = func->chunk;
      func->chunk = NULL;
      if (subChunk) freeChunk(subChunk);
    } else if (obj->type == OBJ_STRING) {
      poolFree(stringPool, obj);
    } else if (!obj->isStatic) {
      freeObject(obj);
    }
  }

  chunk->count = 0;
  chunk->constCount = 0;
}

static int resolveLocal(Compiler *c, const char *name) {
  for (int i = c->localCount - 1; i >= 0; i--)
    if (strcmp(c->locals[i].name, name) == 0)
      return c->locals[i].slot;

  return -1;
}

static int addLocal(Compiler *c, const char *name) {
  if (c->localCount >= MAX_LOCALS) return -1;
  Local *l = &c->locals[c->localCount];
  
  l->name = name; 
  l->len = strlen(name);
  l->slot = c->localCount;

  c->localCount++;

  if (c->localCount > c->maxLocalCount) {
    c->maxLocalCount = c->localCount;
  }

  return l->slot;
}

static void compileNode(ASTNode *node, Compiler *c);
static void compileProgram(ASTNode *node, Compiler *c); 

static inline void emitByte(Compiler *c, uint8_t byte) {
  if (c->posDirty) {
    chunkAddPosEntry(c->chunk, (uint32_t)c->chunk->count, c->posStart, c->posEnd);
    c->posDirty = false;
  }

  chunkWrite(c->chunk, byte);
}

static inline void emitBytes(Compiler *c, uint8_t a, uint8_t b) {
  emitByte(c, a);
  emitByte(c, b);
}

static uint8_t addConst(Compiler *c, Object *obj) {
  int idx = chunkAddConst(c->chunk, obj);

  if (idx < 0 || idx > 255) {
    if (c->err && !*c->err)
      *c->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, c->filename, "Too many constants in chunk (max 256).", c->sourcetext);

    return 0;
  }

  return (uint8_t)idx;
}

static int emitJump(Compiler *c, uint8_t op) {
  emitByte(c, op);
  emitByte(c, 0xFF);
  emitByte(c, 0xFF);

  return (int)c->chunk->count - 2;
}

static void patchJump(Compiler *c, int offset) {
  int jump = (int)c->chunk->count - offset - 2;

  c->chunk->code[offset] = (jump >> 8) & 0xFF;
  c->chunk->code[offset + 1] = jump & 0xFF;
}

static void emitLoop(Compiler *c, int loopStart) {
  emitByte(c, OP_JUMP);

  int jumpBytesPos = (int)c->chunk->count;

  emitByte(c, 0xFF);
  emitByte(c, 0xFF);

  int offset = loopStart - (int)c->chunk->count;

  c->chunk->code[jumpBytesPos] = (offset >> 8) & 0xFF;
  c->chunk->code[jumpBytesPos + 1] = offset & 0xFF;
}

static void compileNumber(ASTNode *node, Compiler *c) {
  NumberNode *num = (NumberNode *)node;

  Object *obj = num->token.type == TOK_FLOAT ? (Object *)initFloat(num->token.val.f) : (Object *)initInt(num->token.val.i);
  setPosFromNode(c, node);
  emitBytes(c, OP_LOAD_CONST, addConst(c, obj));
}

static void compileString(ASTNode *node, Compiler *c) {
  StringNode *str = (StringNode *)node;
  setPosFromNode(c, node);
  emitBytes(c, OP_LOAD_CONST, addStringConst(c, str->token.val.s, str->len));
}

static void compileVarAccess(ASTNode *node, Compiler *c) {
  VarAccessNode *va = (VarAccessNode *)node;

  setPosFromNode(c, node);

  if (c->isFunction) {
    int slot = resolveLocal(c, va->token.val.s);

    if (slot >= 0) {
      emitBytes(c, OP_LOAD_LOCAL, (uint8_t)slot);
      return;
    }

    if (c->funcName && strcmp(va->token.val.s, c->funcName) == 0 && c->funcObj) {
      emitBytes(c, OP_LOAD_CONST, addConst(c, c->funcObj));
      return;
    }
  }

  emitBytes(c, OP_LOAD_VAR, internString(c, va->token.val.s, strlen(va->token.val.s)));
}

static void compileVarAssign(ASTNode *node, Compiler *c) {
  VarAssignNode *va = (VarAssignNode *)node;
  
  setPosFromNode(c, va->value);
  compileNode(va->value, c);

  setPosFromNode(c, node);

  if (c->isFunction) {
    int slot = resolveLocal(c, va->identifier);

    if (slot < 0) {
      slot = addLocal(c, va->identifier);
    }

    if (slot >= 0) {
      emitBytes(c, OP_STORE_LOCAL, (uint8_t)slot);
      return;
    }
  }

  emitBytes(c, OP_STORE_VAR, internString(c, va->identifier, strlen(va->identifier)));
}

static void compileBinOp(ASTNode *node, Compiler *c) {
  BinOpNode *bin = (BinOpNode *)node;
  
  setPosFromNode(c, bin->leftNode);
  compileNode(bin->leftNode, c);

  setPosFromNode(c, bin->rightNode);
  compileNode(bin->rightNode, c);

  setPosFromNode(c, node);

  switch (bin->operTok.type) {
    case TOK_PLUS: emitByte(c, OP_ADD); break;
    case TOK_MINUS: emitByte(c, OP_SUB); break;
    case TOK_MUL: emitByte(c, OP_MUL); break;
    case TOK_DIV: emitByte(c, OP_DIV); break;
    case TOK_POW: emitByte(c, OP_POW); break;
    case TOK_EE: emitByte(c, OP_EQ);  break;
    case TOK_NE: emitByte(c, OP_NE);  break;
    case TOK_LT: emitByte(c, OP_LT);  break;
    case TOK_GT: emitByte(c, OP_GT);  break;
    case TOK_LTE: emitByte(c, OP_LTE); break;
    case TOK_GTE: emitByte(c, OP_GTE); break;
    case TOK_AND: emitByte(c, OP_AND); break;
    case TOK_OR: emitByte(c, OP_OR);  break;
    default: break;
  }
}

static void compileUnaryOp(ASTNode *node, Compiler *c) {
  UnaryOpNode *un = (UnaryOpNode *)node;

  setPosFromNode(c, un->node);
  compileNode(un->node, c);

  setPosFromNode(c, node);

  switch (un->operTok.type) {
    case TOK_MINUS: emitByte(c, OP_NEG); break;
    case TOK_NOT:   emitByte(c, OP_NOT); break;
    /* TOK_PLUS is a no-op: value already on stack */
    default: break;
  }
}

/*
 * IF / ELIF / ELSE
 *
 * OP_JUMP_IF_FALSE pops the condition.
 * Every branch leaves exactly one result on the stack.
 *
 *   <condition>
 *   OP_JUMP_IF_FALSE  -> next
 *   <thenExpr>
 *   OP_JUMP           -> end
 *   [next:] <elifCond|elseExpr|0>
 *   ...
 *   [end:]
 */
static void compileIf(ASTNode *node, Compiler *c) {
  IfNode *n = (IfNode *)node;

  int endJumps[n->elifCount + 2];
  int endJumpCount = 0;

  compileNode(n->condition, c);
  setPosFromNode(c, node);
  int toNext = emitJump(c, OP_JUMP_IF_FALSE);
  
  setPosFromNode(c, n->thenExpr);
  compileNode(n->thenExpr, c);

  setPosFromNode(c, node);
  endJumps[endJumpCount++] = emitJump(c, OP_JUMP);
  patchJump(c, toNext);

  for (size_t i = 0; i < n->elifCount; i++) {
    setPosFromNode(c, n->elifConds[i]);
    compileNode(n->elifConds[i], c);

    setPosFromNode(c, node);
    toNext = emitJump(c, OP_JUMP_IF_FALSE);

    setPosFromNode(c, n->elifExprs[i]);
    compileNode(n->elifExprs[i], c);

    setPosFromNode(c, node);
    endJumps[endJumpCount++] = emitJump(c, OP_JUMP);
    patchJump(c, toNext);
  }

  if (n->elseExpr) {
    setPosFromNode(c, n->elseExpr);
    compileNode(n->elseExpr, c);
  } else {
    setPosFromNode(c, node);
    emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)initInt(0)));
  }

  for (int i = 0; i < endJumpCount; i++) 
    patchJump(c, endJumps[i]);
}

/*
 * WHILE
 *
 *   [loopStart:]
 *   <condition>
 *   OP_JUMP_IF_FALSE  -> exit
 *   <body>
 *   OP_POP
 *   OP_JUMP           -> loopStart
 *   [exit:]
 *   OP_LOAD_CONST 1
 */
static void compileWhile(ASTNode *node, Compiler *c) {
  WhileNode *wn = (WhileNode *)node;

  int loopStart = (int)c->chunk->count;
  
  setPosFromNode(c, wn->condition);
  compileNode(wn->condition, c);

  setPosFromNode(c, node);
  int exitJump = emitJump(c, OP_JUMP_IF_FALSE);

  LoopInfo info = { .start = loopStart, .breaks = NULL, .continues = NULL, .next = c->loop };
  c->loop = &info;
  
  setPosFromNode(c, wn->body);
  compileNode(wn->body, c);

  setPosFromNode(c, node);
  emitByte(c, OP_POP);

  /* Patch continues to the back-edge (re-evaluate condition). */
  JumpList *cl = info.continues;

  while (cl) {
    patchJump(c, cl->offset);
    cl = cl->next;
  }

  c->loop = info.next;
  
  setPosFromNode(c, node);
  emitLoop(c, loopStart);
  patchJump(c, exitJump);

  JumpList *bl = info.breaks;
  while (bl) { patchJump(c, bl->offset); bl = bl->next; }

  emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)initInt(1)));
}

/*
 * FOR
 *
 *   <iterable>
 *   OP_FOR_PREP
 *   [loopStart:]
 *   OP_FOR_ITER       -> exit
 *   OP_STORE_VAR <loopVar>
 *   OP_POP
 *   <body>
 *   OP_POP
 *   [continueTarget:]
 *   OP_JUMP           -> loopStart
 *   [exit:]
 *   OP_POP  (index)
 *   OP_POP  (length)
 *   OP_POP  (iterable)
 *   OP_LOAD_CONST 1
 */
static void compileFor(ASTNode *node, Compiler *c) {
  ForNode *fn = (ForNode *)node;
  
  setPosFromNode(c, fn->iterable);
  compileNode(fn->iterable, c);

  setPosFromNode(c, node);
  emitByte(c, OP_FOR_PREP);

  int loopStart = (int)c->chunk->count;
  int exitJump = emitJump(c, OP_FOR_ITER);
  
  emitBytes(c, OP_STORE_VAR, internString(c, fn->ident.val.s, strlen(fn->ident.val.s)));
  emitByte(c, OP_POP);

  LoopInfo info = { .start = loopStart, .breaks = NULL, .continues = NULL, .next = c->loop };
  c->loop = &info;

  compileNode(fn->body, c);
  emitByte(c, OP_POP);

  /* Patch continues to the back-edge (re-run FOR_ITER). */
  JumpList *cl = info.continues;

  while (cl) {
    patchJump(c, cl->offset);
    cl = cl->next;
  }

  c->loop = info.next;

  emitLoop(c, loopStart);
  patchJump(c, exitJump);

  JumpList *bl = info.breaks;

  while (bl) { 
    patchJump(c, bl->offset);
    bl = bl->next;
  }


  emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)initInt(1)));
}

/*
 * FUNCTION DEFINITION
 *
 *   OP_LOAD_CONST  <Function>
 *   OP_STORE_VAR   <name>       (peek)
 *   OP_POP
 *   OP_LOAD_CONST  1
 */
static void compileFunction(ASTNode *node, Compiler *c) {
  FunctionNode *fn = (FunctionNode *)node;
  Function *func = initFunction(fn);

  // compile body with a fresh compiler marked as function scope
  Compiler fc = {0};
  fc.chunk = initChunk();
  fc.err = c->err;
  fc.filename = c->filename;
  fc.sourcetext = c->sourcetext;
  fc.isFunction = true;
  fc.funcName = fn->name;
  fc.funcObj = (Object*)func;

  fc.chunk->filename = c->filename;
  fc.chunk->sourcetext = c->sourcetext;
  
  setPosFromNode(&fc, fn->body);

  // pre-declare parameters as locals in order
  for (size_t i = 0; i < fn->paramCount; i++)
    addLocal(&fc, fn->params[i]);

  if (fn->body->type == NODE_PROGRAM)
    compileProgram(fn->body, &fc);
  else
    compileNode(fn->body, &fc);
  
  setPosFromNode(&fc, node);
  emitByte(&fc, OP_HALT);

  func->chunk = fc.chunk;
  func->chunk->maxLocals = fc.maxLocalCount;
  func->maxLocals = fc.maxLocalCount;

  setPosFromNode(c, node);
  emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)func));
  emitBytes(c, OP_STORE_VAR, internString(c, fn->name, strlen(fn->name)));
  emitByte(c, OP_POP);

  setPosFromNode(c, node);
  emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)initInt(1)));
}

/*
 * TRY / CATCH
 *
 *   OP_TRY_PUSH       -> catch
 *   <body>
 *   OP_TRY_POP
 *   OP_JUMP           -> end
 *   [catch:]          ; error string on stack
 *   OP_STORE_VAR <errVar>   (peek)
 *   OP_POP
 *   <errHandler>
 *   [end:]
 */
static void compileTryCatch(ASTNode *node, Compiler *c) {
  TryCatchNode *tn = (TryCatchNode *)node;
  
  setPosFromNode(c, node);
  int catchJump = emitJump(c, OP_TRY_PUSH);
  
  setPosFromNode(c, tn->body);
  compileNode(tn->body, c);

  setPosFromNode(c, node);
  emitByte(c, OP_TRY_POP);
  
  setPosFromNode(c, node);
  int endJump = emitJump(c, OP_JUMP);

  setPosFromNode(c, node);
  patchJump(c, catchJump);
  
  setPosFromNode(c, node);
  emitBytes(c, OP_STORE_VAR, internString(c, tn->errIdentifier.val.s, strlen(tn->errIdentifier.val.s)));

  setPosFromNode(c, node);
  emitByte(c, OP_POP);
  
  setPosFromNode(c, tn->errHandler);
  compileNode(tn->errHandler, c);
  
  setPosFromNode(c, node);
  patchJump(c, endJump);
}

static void compileImport(ASTNode *node, Compiler *c) {
  ImportNode *in = (ImportNode *)node;
  setPosFromNode(c, node);
  emitBytes(c, OP_IMPORT, internString(c, in->filePath.val.s, strlen(in->filePath.val.s)));
}

static void compileReturn(ASTNode *node, Compiler *c) {
  ReturnNode *ret = (ReturnNode *)node;

  setPosFromNode(c, ret->expr);
  compileNode(ret->expr, c);

  setPosFromNode(c, node);

  emitByte(c, OP_RETURN);
}

static void compileList(ASTNode *node, Compiler *c) {
  ListNode *ln = (ListNode *)node;
  setPosFromNode(c, node);

  for (uint64_t i = 0; i < ln->size; i++) {
    setPosFromNode(c, node);
    compileNode(ln->objects[i], c);
  }
  
  setPosFromNode(c, node);
  emitBytes(c, OP_BUILD_LIST, (uint8_t)ln->size);
}

static void compileIndex(ASTNode *node, Compiler *c) {
  IndexNode *in = (IndexNode *)node;

  setPosFromNode(c, in->target);
  compileNode(in->target, c);
  
  setPosFromNode(c, node);
  
  setPosFromNode(c, in->index);
  compileNode(in->index, c);

  setPosFromNode(c, node);

  emitByte(c, OP_INDEX_GET);
}

static void compileIndexAssign(ASTNode *node, Compiler *c) {
  IndexAssignNode *ia = (IndexAssignNode *)node;
  
  setPosFromNode(c, ia->target);
  compileNode(ia->target, c);

  setPosFromNode(c, ia->index);
  compileNode(ia->index, c);
  
  setPosFromNode(c, ia->value);
  compileNode(ia->value, c);

  setPosFromNode(c, node);
  emitByte(c, OP_INDEX_SET);
}

static void compileFunctionCall(ASTNode *node, Compiler *c) {
  FunctionCallNode *fc = (FunctionCallNode *)node;

  setPosFromNode(c, fc->callee);
  compileNode(fc->callee, c);

  for (size_t i = 0; i < fc->argCount; i++) {
    setPosFromNode(c, fc->args[i]);
    compileNode(fc->args[i], c);
  }
  
  setPosFromNode(c, node);
  emitBytes(c, OP_CALL, (uint8_t)fc->argCount);
}

static void compilePropertyAssignNode(ASTNode* node, Compiler* c) {
  PropertyAssignNode* pa = (PropertyAssignNode*)node;

  setPosFromNode(c, pa->target);
  compileNode(pa->target, c);   

  setPosFromNode(c, pa->value);
  compileNode(pa->value, c);

  setPosFromNode(c, node);
  emitBytes(c, OP_PROPERTY_SET, internString(c, pa->field.val.s, strlen(pa->field.val.s)));
}

/*
 * PROGRAM
 *
 * All statements except the last are popped. The last result stays
 * on the stack for OP_HALT (or the caller in a function body).
 */
static void compileProgram(ASTNode *node, Compiler *c) {
  ProgramNode *prog = (ProgramNode *)node;

  if (prog->count == 0) {
    setPosFromNode(c, node);
    emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)initInt(0)));
    return;
  }

  for (size_t i = 0; i < prog->count; i++) {
    compileNode(prog->statements[i], c);
    setPosFromNode(c, prog->statements[i]);

    if (i < prog->count - 1) 
      emitByte(c, OP_POP);
  }
}

static void compileClass(ASTNode *node, Compiler *c) {
  ClassNode *cn = (ClassNode *)node;
  Class *class = initClass(cn);

  Compiler cc = {0};

  cc.chunk = initChunk(),
  cc.err = c->err,
  cc.filename = c->filename,
  cc.sourcetext = c->sourcetext,

  cc.chunk->filename = c->filename;
  cc.chunk->sourcetext = c->sourcetext;

  setPosFromNode(&cc, cn->body);

  if (cn->body->type == NODE_PROGRAM)
    compileProgram(cn->body, &cc);
  else
    compileNode(cn->body, &cc);

  setPosFromNode(&cc, node);
  emitByte(&cc, OP_HALT);

  class->chunk = cc.chunk;
  class->maxLocals = cc.maxLocalCount;

  setPosFromNode(c, node);
  emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)class));

  emitBytes(c, OP_STORE_VAR, internString(c, class->name, strlen(class->name)));
  emitByte(c, OP_POP);

  setPosFromNode(c, node);
  emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)initInt(1)));
}

static void compilePropertyAccessNode(ASTNode* node, Compiler* c) {
  PropertyAccessNode* pa = (PropertyAccessNode*)node;

  setPosFromNode(c, pa->target);
  compileNode(pa->target, c);
  
  setPosFromNode(c, node);
  emitBytes(c, OP_PROPERTY_ACCESS, internString(c, pa->field.val.s, strlen(pa->field.val.s)));
}

static void compileNode(ASTNode *node, Compiler *c) {
  if (!node || *c->err) return;

  switch (node->type) {
    case NODE_NUMBER: compileNumber(node, c); break;
    case NODE_STRING: compileString(node, c); break;
    case NODE_LIST: compileList(node, c); break;
    case NODE_INDEX: compileIndex(node, c); break;
    case NODE_INDEXASSIGN: compileIndexAssign(node, c); break;
    case NODE_VARACCESS: compileVarAccess(node, c); break;
    case NODE_VARASSIGN: compileVarAssign(node, c); break;
    case NODE_BINOP: compileBinOp(node, c); break;
    case NODE_UNARYOP: compileUnaryOp(node, c); break;
    case NODE_IF: compileIf(node, c); break;
    case NODE_WHILE: compileWhile(node, c); break;
    case NODE_FOR: compileFor(node, c); break;
    case NODE_FUNCTION: compileFunction(node, c); break;
    case NODE_FUNCTION_CALL: compileFunctionCall(node, c); break;
    case NODE_TRYCATCH: compileTryCatch(node, c); break;
    case NODE_IMPORT: compileImport(node, c); break;
    case NODE_RETURN: compileReturn(node, c); break;
    case NODE_CLASS: compileClass(node, c); break;
    case NODE_PROPERTYACCESS: compilePropertyAccessNode(node, c); break;
    case NODE_PROPERTYASSIGN: compilePropertyAssignNode(node, c); break;
    case NODE_BREAK:
      if (c->loop) {
        setPosFromNode(c, node);
        int j = emitJump(c, OP_JUMP);
        JumpList *bl = arenaAlloc(objectArena, sizeof(JumpList));
        bl->offset = j; bl->next = c->loop->breaks; c->loop->breaks = bl;
      }
      break;
    case NODE_CONTINUE:
      if (c->loop) {
        setPosFromNode(c, node);
        int j = emitJump(c, OP_JUMP);
        JumpList *cl = arenaAlloc(objectArena, sizeof(JumpList));
        cl->offset = j; cl->next = c->loop->continues; c->loop->continues = cl;
      }

      break;
    case NODE_PROGRAM: compileProgram(node, c); break;
    default: break;
  }
}

Chunk *compileAST(ASTNode *ast, Error **err, char *filename, char *sourcetext) {
  Compiler c = {
    .chunk = initChunk(),
    .err = err,
    .filename = filename,
    .sourcetext = sourcetext,
    .maxLocalCount = 0,
    .loop = NULL
  };

  if (!c.chunk) return NULL;

  c.chunk->filename = filename;
  c.chunk->sourcetext = sourcetext;

  if (ast->type == NODE_PROGRAM) 
    compileProgram(ast, &c);
  else                           
    compileNode(ast, &c);
  
  setPos(&c, getNodeStart(ast), getNodeEnd(ast));
  emitByte(&c, OP_HALT);

  c.chunk->maxLocals = c.maxLocalCount;

  if (*err) { 
    freeChunk(c.chunk);
    return NULL;
  }

  return c.chunk;
}

static void printConstant(Chunk *chunk, uint8_t idx) {
  if (idx >= chunk->constCount) { 
    printf(" <invalid const %u>", idx);
    return;
  }

  Object *obj = chunk->constants[idx];

  switch (obj->type) {
    case OBJ_NUMBER_INT: printf(" %" PRId64, ((Number*)obj)->as.i); break;
    case OBJ_NUMBER_FLOAT: printf(" %f", ((Number*)obj)->as.f); break;
    case OBJ_STRING: printf(" \"%s\"", ((String*)obj)->value); break;
    default: printf(" <obj>"); break;
  }
}

void disassembleChunk(Chunk *chunk, const char *name) {
  printf("== Chunk: %s ==\n", name);

  for (size_t i = 0; i < chunk->count; i++) {
    printf("%04zu  ", i);
    uint8_t op = chunk->code[i];

    switch (op) {
      case OP_LOAD_CONST: printf("OP_LOAD_CONST"); printConstant(chunk, chunk->code[++i]); printf("\n"); break;
      case OP_LOAD_VAR: printf("OP_LOAD_VAR"); printConstant(chunk, chunk->code[++i]); printf("\n"); break;
      case OP_STORE_VAR: printf("OP_STORE_VAR"); printConstant(chunk, chunk->code[++i]); printf("\n"); break;
      case OP_ADD: printf("OP_ADD\n"); break;
      case OP_SUB: printf("OP_SUB\n"); break;
      case OP_MUL: printf("OP_MUL\n"); break;
      case OP_DIV: printf("OP_DIV\n"); break;
      case OP_POW: printf("OP_POW\n"); break;
      case OP_EQ: printf("OP_EQ\n"); break;
      case OP_NE: printf("OP_NE\n"); break;
      case OP_LT: printf("OP_LT\n"); break;
      case OP_GT: printf("OP_GT\n"); break;
      case OP_LTE: printf("OP_LTE\n"); break;
      case OP_GTE: printf("OP_GTE\n"); break;
      case OP_AND: printf("OP_AND\n"); break;
      case OP_OR: printf("OP_OR\n"); break;
      case OP_NOT: printf("OP_NOT\n"); break;
      case OP_NEG: printf("OP_NEG\n"); break;
      case OP_INDEX_GET: printf("OP_INDEX_GET\n"); break;
      case OP_INDEX_SET: printf("OP_INDEX_SET\n"); break;
      case OP_BUILD_LIST: printf("OP_BUILD_LIST %u\n", chunk->code[++i]); break;
      case OP_POP: printf("OP_POP\n"); break;
      case OP_FOR_PREP: printf("OP_FOR_PREP\n"); break;

      case OP_FOR_ITER: {
        uint8_t hi = chunk->code[++i], lo = chunk->code[++i];
        printf("OP_FOR_ITER %d\n", (int16_t)((hi << 8) | lo));
        break;
      }

      case OP_TRY_PUSH: {
        uint8_t hi = chunk->code[++i], lo = chunk->code[++i];
        printf("OP_TRY_PUSH %d\n", (int16_t)((hi << 8) | lo));
        break;
      }

      case OP_TRY_POP: printf("OP_TRY_POP\n"); break;
      case OP_IMPORT: printf("OP_IMPORT"); printConstant(chunk, chunk->code[++i]); printf("\n"); break;
      case OP_CALL: printf("OP_CALL %u\n", chunk->code[++i]); break;
      case OP_BREAK: printf("OP_BREAK\n"); break;
      case OP_CONTINUE: printf("OP_CONTINUE\n"); break;
      case OP_RETURN: printf("OP_RETURN\n"); break;
      case OP_HALT: printf("OP_HALT\n"); break;
      case OP_LOAD_LOCAL:  printf("OP_LOAD_LOCAL  %u\n", chunk->code[++i]); break;
      case OP_STORE_LOCAL: printf("OP_STORE_LOCAL %u\n", chunk->code[++i]); break;
      case OP_PROPERTY_ACCESS: printf("OP_PROPERTY_ACCESS"); printConstant(chunk, chunk->code[++i]); printf("\n"); break;
      case OP_PROPERTY_SET: printf("OP_PROPERTY_ASSIGN"); printConstant(chunk, chunk->code[++i]); printf("\n"); break;

      case OP_JUMP: {
        uint8_t hi = chunk->code[++i], lo = chunk->code[++i];
        printf("OP_JUMP %d\n", (int16_t)((hi << 8) | lo)); break;
      }

      case OP_JUMP_IF_FALSE: {
        uint8_t hi = chunk->code[++i], lo = chunk->code[++i];
        printf("OP_JUMP_IF_FALSE %d\n", (int16_t)((hi << 8) | lo)); break;
      }

      default: printf("UNKNOWN_OPCODE %u\n", op); break;
    }
  }

  printf("== End Chunk ==\n");
}
