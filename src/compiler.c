#include "../include/compiler.h"
#include "../include/memarena.h"
#include "../include/node.h"
#include "../include/object.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>

#define CHUNK_INIT_CAP 256
#define CONST_INIT_CAP 64

Chunk *initChunk(void) {
  Chunk *chunk = arenaNew(objectArena, Chunk);
  if (!chunk) return NULL;

  chunk->code = arenaAlloc(objectArena, CHUNK_INIT_CAP * sizeof(uint8_t));
  if (!chunk->code) return NULL;

  chunk->count = 0; 
  chunk->capacity = CHUNK_INIT_CAP;

  chunk->constants = arenaAlloc(objectArena, CONST_INIT_CAP * sizeof(Object *));
  if (!chunk->constants) return NULL;

  chunk->constCount = 0; chunk->constCapacity = CONST_INIT_CAP;

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

  if (chunk->constCount >= chunk->constCapacity) {
    size_t oldCap = chunk->constCapacity;
    size_t newCap = oldCap * 2;

    Object **grown = arenaRealloc(objectArena, chunk->constants, oldCap * sizeof(Object *), newCap * sizeof(Object *));
    if (!grown) return -1;

    chunk->constants = grown; chunk->constCapacity = newCap;
  }

  int idx = (int)chunk->constCount;
  chunk->constants[chunk->constCount++] = obj;

  return idx;
}

void freeChunk(Chunk *chunk) {
  if (!chunk) return;

  for (size_t i = 0; i < chunk->constCount; i++) 
    freeObject(chunk->constants[i]);

  chunk->count = 0; chunk->constCount = 0;
}

static void compileNode(ASTNode *node, Compiler *c);

static inline void emitByte(Compiler *c, uint8_t byte) {
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
  emitBytes(c, OP_LOAD_CONST, addConst(c, obj));
}

static void compileString(ASTNode *node, Compiler *c) {
  StringNode *str = (StringNode *)node;
  Object *obj = (Object *)initString(str->token.val.s, str->len);
  emitBytes(c, OP_LOAD_CONST, addConst(c, obj));
}

static void compileVarAccess(ASTNode *node, Compiler *c) {
  VarAccessNode *va = (VarAccessNode *)node;
  Object *name = (Object *)initString(va->token.val.s, strlen(va->token.val.s));
  emitBytes(c, OP_LOAD_VAR, addConst(c, name));
}

static void compileVarAssign(ASTNode *node, Compiler *c) {
  VarAssignNode *va = (VarAssignNode *)node;
  compileNode(va->value, c);
  Object *name = (Object *)initString(va->identifier, strlen(va->identifier));
  emitBytes(c, OP_STORE_VAR, addConst(c, name));
}

static void compileBinOp(ASTNode *node, Compiler *c) {
  BinOpNode *bin = (BinOpNode *)node;

  compileNode(bin->leftNode, c);
  compileNode(bin->rightNode, c);

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
  compileNode(un->node, c);

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
  int toNext = emitJump(c, OP_JUMP_IF_FALSE);

  compileNode(n->thenExpr, c);
  endJumps[endJumpCount++] = emitJump(c, OP_JUMP);
  patchJump(c, toNext);

  for (size_t i = 0; i < n->elifCount; i++) {
    compileNode(n->elifConds[i], c);
    toNext = emitJump(c, OP_JUMP_IF_FALSE);
    compileNode(n->elifExprs[i], c);
    endJumps[endJumpCount++] = emitJump(c, OP_JUMP);
    patchJump(c, toNext);
  }

  if (n->elseExpr) {
    compileNode(n->elseExpr, c);
  } else {
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

  compileNode(wn->condition, c);
  int exitJump = emitJump(c, OP_JUMP_IF_FALSE);

  LoopInfo info = { .start = loopStart, .breaks = NULL, .continues = NULL, .next = c->loop };
  c->loop = &info;

  compileNode(wn->body, c);
  emitByte(c, OP_POP);

  /* Patch continues to the back-edge (re-evaluate condition). */
  JumpList *cl = info.continues;

  while (cl) {
    patchJump(c, cl->offset);
    cl = cl->next;
  }

  c->loop = info.next;

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

  compileNode(fn->iterable, c);
  emitByte(c, OP_FOR_PREP);

  int loopStart = (int)c->chunk->count;
  int exitJump = emitJump(c, OP_FOR_ITER);

  Object *name = (Object *)initString(fn->ident.val.s, strlen(fn->ident.val.s));
  emitBytes(c, OP_STORE_VAR, addConst(c, name));
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

  emitByte(c, OP_POP);   /* index    */
  emitByte(c, OP_POP);   /* length   */
  emitByte(c, OP_POP);   /* iterable */

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

  func->chunk = compileAST(fn->body, c->err, c->filename, c->sourcetext);

  emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)func));

  Object *name = (Object *)initString(fn->name, strlen(fn->name));
  emitBytes(c, OP_STORE_VAR, addConst(c, name));

  emitByte(c, OP_POP);
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

  int catchJump = emitJump(c, OP_TRY_PUSH);

  compileNode(tn->body, c);
  emitByte(c, OP_TRY_POP);

  int endJump = emitJump(c, OP_JUMP);
  patchJump(c, catchJump);

  Object *name = (Object *)initString(tn->errIdentifier.val.s, strlen(tn->errIdentifier.val.s));
  emitBytes(c, OP_STORE_VAR, addConst(c, name));
  emitByte(c, OP_POP);

  compileNode(tn->errHandler, c);

  patchJump(c, endJump);
}

static void compileImport(ASTNode *node, Compiler *c) {
  ImportNode *in = (ImportNode *)node;
  Object *path = (Object *)initString(in->filePath.val.s, strlen(in->filePath.val.s));
  emitBytes(c, OP_IMPORT, addConst(c, path));
}

static void compileReturn(ASTNode *node, Compiler *c) {
  ReturnNode *ret = (ReturnNode *)node;
  compileNode(ret->expr, c);
  emitByte(c, OP_RETURN);
}

static void compileList(ASTNode *node, Compiler *c) {
  ListNode *ln = (ListNode *)node;

  for (uint64_t i = 0; i < ln->size; i++) 
    compileNode(ln->objects[i], c);

  emitBytes(c, OP_BUILD_LIST, (uint8_t)ln->size);
}

static void compileIndex(ASTNode *node, Compiler *c) {
  IndexNode *in = (IndexNode *)node;
  compileNode(in->target, c);
  compileNode(in->index, c);
  emitByte(c, OP_INDEX_GET);
}

static void compileIndexAssign(ASTNode *node, Compiler *c) {
  IndexAssignNode *ia = (IndexAssignNode *)node;
  Object *name = (Object *)initString(ia->targetIdent.val.s, strlen(ia->targetIdent.val.s));
  compileNode(ia->index, c);
  compileNode(ia->value, c);
  emitBytes(c, OP_STORE_INDEX, addConst(c, name));
}

static void compileFunctionCall(ASTNode *node, Compiler *c) {
  FunctionCallNode *fc = (FunctionCallNode *)node;
  compileNode(fc->callee, c);

  for (size_t i = 0; i < fc->argCount; i++) {
    compileNode(fc->args[i], c);
  }

  emitBytes(c, OP_CALL, (uint8_t)fc->argCount);
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
    emitBytes(c, OP_LOAD_CONST, addConst(c, (Object *)initInt(0)));
    return;
  }

  for (size_t i = 0; i < prog->count; i++) {
    compileNode(prog->statements[i], c);

    if (i < prog->count - 1) 
      emitByte(c, OP_POP);
  }
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
    case NODE_BREAK:
      if (c->loop) {
        int j = emitJump(c, OP_JUMP);
        JumpList *bl = arenaAlloc(objectArena, sizeof(JumpList));
        bl->offset = j; bl->next = c->loop->breaks; c->loop->breaks = bl;
      }
      break;
    case NODE_CONTINUE:
      if (c->loop) {
        int j = emitJump(c, OP_JUMP);
        JumpList *cl = arenaAlloc(objectArena, sizeof(JumpList));
        cl->offset = j; cl->next = c->loop->continues; c->loop->continues = cl;
      }

      break;
    case NODE_PROGRAM:       compileProgram(node, c);      break;
    default: break;
  }
}

Chunk *compileAST(ASTNode *ast, Error **err, char *filename, char *sourcetext) {
  Compiler c = {
    .chunk = initChunk(),
    .err = err,
    .filename = filename,
    .sourcetext = sourcetext,
    .loop = NULL
  };

  if (!c.chunk) return NULL;

  if (ast->type == NODE_PROGRAM) 
    compileProgram(ast, &c);
  else                           
    compileNode(ast, &c);

  emitByte(&c, OP_HALT);

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
      case OP_STORE_INDEX: printf("OP_STORE_INDEX"); printConstant(chunk, chunk->code[++i]); printf("\n"); break;
      case OP_CALL: printf("OP_CALL %u\n", chunk->code[++i]); break;
      case OP_BREAK: printf("OP_BREAK\n"); break;
      case OP_CONTINUE: printf("OP_CONTINUE\n"); break;
      case OP_RETURN: printf("OP_RETURN\n"); break;
      case OP_HALT: printf("OP_HALT\n"); break;

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
