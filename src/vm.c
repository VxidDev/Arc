#include "../include/vm.h"
#include "../include/object.h"
#include "../include/memarena.h"
#include "../include/utils.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/compiler.h"
#include "../include/node.h"
#include "../include/repl/readfile.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static inline void push(VM *vm, Object *obj) {
    if (vm->stackTop >= VM_STACK_MAX) return; 
    vm->stack[vm->stackTop++] = obj;
}

static inline Object *pop(VM *vm) {
  if (vm->stackTop <= 0) return NULL;
  return vm->stack[--vm->stackTop];
}

static inline Object *peek(VM *vm, int distance) {
  if (vm->stackTop - 1 - distance < 0) return NULL;
  return vm->stack[vm->stackTop - 1 - distance];
}

#define READ_BYTE(vm) (*((vm)->frames[(vm)->frameTop - 1].ip++))
#define READ_CONST(vm) ((vm)->frames[(vm)->frameTop - 1].chunk->constants[READ_BYTE(vm)])
#define READ_SHORT(vm) (((vm)->frames[(vm)->frameTop - 1].ip += 2), (int16_t)(((vm)->frames[(vm)->frameTop - 1].ip[-2] << 8) | (vm)->frames[(vm)->frameTop - 1].ip[-1]))

static Object *doArith(VM *vm, OpCode op, Object *a, Object *b) {
  (void)vm;
  ObjType at = a->type;
  ObjType bt = b->type;
  bool aNum = at == OBJ_NUMBER_INT || at == OBJ_NUMBER_FLOAT;
  bool bNum = bt == OBJ_NUMBER_INT || bt == OBJ_NUMBER_FLOAT;
  if (at == OBJ_STRING || bt == OBJ_STRING) {
    if (op == OP_ADD && at == OBJ_STRING && bt == OBJ_STRING) return (Object *)addString((String *)a, (String *)b);
    if (op == OP_MUL && at == OBJ_STRING && bt == OBJ_NUMBER_INT) return (Object *)mulString((String *)a, (Number *)b);
    if (op == OP_EQ) return (Object *)initInt(at == OBJ_STRING && bt == OBJ_STRING && strcmp(((String *)a)->value, ((String *)b)->value) == 0);
    if (op == OP_NE) return (Object *)initInt(!(at == OBJ_STRING && bt == OBJ_STRING && strcmp(((String *)a)->value, ((String *)b)->value) == 0));
    return NULL;
  }
  if (!aNum || !bNum) return NULL;
  Number *dest = copyNumber((Number *)b);
  Number *src  = (Number *)a;
  ErrType result;
  switch (op) {
    case OP_ADD: result = addNumber(dest, src); break;
    case OP_SUB: result = subNumber(dest, src); break;
    case OP_MUL: result = mulNumber(dest, src); break;
    case OP_DIV: result = divNumber(dest, src); break;
    case OP_POW: result = powNumber(dest, src); break;
    case OP_EQ: result = isEqualNumber(dest, src); break;
    case OP_NE: result = isNotEqualNumber(dest, src); break;
    case OP_LT: result = isLessThanNumber(dest, src); break;
    case OP_GT: result = isGreaterThanNumber(dest, src); break;
    case OP_LTE: result = isLessThanEqualNumber(dest, src); break;
    case OP_GTE: result = isGreaterThanEqualNumber(dest, src); break;
    case OP_AND: result = andNumber(dest, src); break;
    case OP_OR: result = orNumber(dest, src); break;
    default: result = ERR_TYPE;
  }
  if (result != ERR_NONE) {
    if (!*vm->err) {
      if (result == ERR_DIV_BY_ZERO)
        *vm->err = initValueError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Division by zero.", vm->sourcetext);
      else if (result == ERR_TYPE)
        *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Incompatible types for operation.", vm->sourcetext);
      else if (result == ERR_NULL)
        *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Null operand in arithmetic operation.", vm->sourcetext);
    }
    freeObject(a);
    freeObject(b);
    freeObject((Object *)dest);
    return NULL;
  }
  freeObject(a);
  freeObject(b);
  return (Object *)dest;
}

#define HANDLE_ERROR() \
  if (vm->tryStackTop > 0) { \
    vm->frames[vm->frameTop - 1].ip = vm->tryStack[--vm->tryStackTop]; \
    if (*vm->err) { \
      Object *errStr = (Object *)initString((*vm->err)->details, strlen((*vm->err)->details)); \
      push(vm, errStr); \
      freeError(*vm->err); \
      *vm->err = NULL; \
    } else { \
      push(vm, (Object *)initString("Unknown Error", 13)); \
    } \
    continue; \
  } else { \
    return NULL; \
  }

Object *vmRun(VM *vm) {
  for (;;) {
    uint8_t op = READ_BYTE(vm);
    switch (op) {
      case OP_LOAD_CONST: push(vm, copyObject(READ_CONST(vm))); break;
      case OP_LOAD_VAR: {
        String *name = (String *)READ_CONST(vm);
        Object *val  = getTable(vm->frames[vm->frameTop - 1].variables, name->value);
        if (!val) {
          char buf[256]; snprintf(buf, sizeof(buf), "Undefined variable \"%s\".", name->value);
          if (!*vm->err) *vm->err = initNameError((Position){0,0,0}, (Position){0,0,0}, vm->filename, buf, vm->sourcetext);
          HANDLE_ERROR();
        }
        push(vm, copyObject(val));
        break;
      }
      case OP_STORE_VAR: {
        String *name = (String *)READ_CONST(vm);
        Object *val  = peek(vm, 0);
        setTable(vm->frames[vm->frameTop - 1].variables, name->value, val, true);
        break;
      }
      case OP_POP: { Object *o = pop(vm); if (o) freeObject(o); break; }
      case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV:
      case OP_POW: case OP_EQ:  case OP_NE:  case OP_LT:
      case OP_GT:  case OP_LTE: case OP_GTE: case OP_AND: case OP_OR: {
        Object *b = pop(vm); Object *a = pop(vm);
        Object *res = doArith(vm, op, a, b);
        if (!res) {
          if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Incompatible types for operation", vm->sourcetext);
          HANDLE_ERROR();
        }
        push(vm, res);
        break;
      }
      case OP_NEG: {
        Object *a = pop(vm);
        if (a->type == OBJ_NUMBER_INT) push(vm, (Object *)initInt(-((Number *)a)->as.i));
        else if (a->type == OBJ_NUMBER_FLOAT) push(vm, (Object *)initFloat(-((Number *)a)->as.f));
        else { freeObject(a); HANDLE_ERROR(); }
        freeObject(a); break;
      }
      case OP_NOT: {
        Object *a = pop(vm);
        if (a->type == OBJ_NUMBER_INT) push(vm, (Object *)initInt(!((Number *)a)->as.i));
        else if (a->type == OBJ_NUMBER_FLOAT) push(vm, (Object *)initFloat(!((Number *)a)->as.f));
        else { freeObject(a); HANDLE_ERROR(); }
        freeObject(a); break;
      }
      case OP_JUMP: { int16_t offset = READ_SHORT(vm); vm->frames[vm->frameTop - 1].ip += offset; break; }
      case OP_JUMP_IF_FALSE: {
        int16_t offset = READ_SHORT(vm);
        Object *cond = pop(vm);
        if (cond->type != OBJ_NUMBER_INT) {
          if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Condition must be an integer.", vm->sourcetext);
          freeObject(cond); HANDLE_ERROR();
        }
        if (((Number *)cond)->as.i == 0) vm->frames[vm->frameTop - 1].ip += offset;
        freeObject(cond);
        break;
      }
      case OP_FOR_PREP: {
        Object *iterable = pop(vm);
        if (iterable->type != OBJ_LIST && iterable->type != OBJ_STRING) { 
           if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Object is not iterable", vm->sourcetext);
           freeObject(iterable); HANDLE_ERROR(); 
        }
        push(vm, iterable);
        push(vm, (Object *)initInt(iterable->type == OBJ_LIST ? ((List *)iterable)->size : ((String *)iterable)->len));
        push(vm, (Object *)initInt(0)); // index
        break;
      }
      case OP_FOR_ITER: {
        int16_t offset = READ_SHORT(vm);
        Number *index = (Number *)peek(vm, 0);
        Number *length = (Number *)peek(vm, 1);
        Object *iterable = peek(vm, 2);
        if (index->as.i < length->as.i) {
          Object *item;
          if (iterable->type == OBJ_LIST) item = copyObject(((List *)iterable)->objects[index->as.i]);
          else { char buf[2] = { ((String *)iterable)->value[index->as.i], '\0' }; item = (Object *)initString(buf, 1); }
          push(vm, item);
          index->as.i++;
        } else {
          vm->frames[vm->frameTop - 1].ip += offset;
        }
        break;
      }
      case OP_BUILD_LIST: {
        uint8_t count = READ_BYTE(vm);
        Object *items[count];
        for (int i = count - 1; i >= 0; i--) items[i] = pop(vm);
        push(vm, (Object *)initList(items, count, count));
        break;
      }
      case OP_INDEX_GET: {
        Object *idx = pop(vm); Object *target = pop(vm);
        if (idx->type != OBJ_NUMBER_INT) { 
          if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index must be an integer.", vm->sourcetext);
          freeObject(idx); freeObject(target); HANDLE_ERROR(); 
        }
        int64_t i = ((Number *)idx)->as.i;
        if (target->type == OBJ_STRING) {
          String *str = (String *)target;
          if (i < 0 || (uint64_t)i >= str->len) { 
            if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range.", vm->sourcetext);
            freeObject(idx); freeObject(target); HANDLE_ERROR(); 
          }
          char buf[2] = { str->value[i], '\0' }; push(vm, (Object *)initString(buf, 1));
        } else if (target->type == OBJ_LIST) {
          List *list = (List *)target;
          if (i < 0 || (uint64_t)i >= list->size) { 
            if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range.", vm->sourcetext);
            freeObject(idx); freeObject(target); HANDLE_ERROR(); 
          }
          push(vm, copyObject(list->objects[i]));
        } else { 
          if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Target is not indexable.", vm->sourcetext);
          freeObject(idx); freeObject(target); HANDLE_ERROR(); 
        }
        freeObject(idx); freeObject(target); break;
      }
      case OP_INDEX_SET: {
        Object *val = pop(vm); Object *idx = pop(vm); Object *target = pop(vm);
        if (idx->type != OBJ_NUMBER_INT) { 
          if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index must be an integer.", vm->sourcetext);
          freeObject(val); freeObject(idx); freeObject(target); HANDLE_ERROR(); 
        }
        int64_t i = ((Number *)idx)->as.i;
        if (target->type == OBJ_LIST) {
          List *list = (List *)target;
          if (i >= 0 && (uint64_t)i < list->size) { freeObject(list->objects[i]); list->objects[i] = val; }
          else { 
            if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range.", vm->sourcetext);
            freeObject(val); freeObject(idx); freeObject(target); HANDLE_ERROR(); 
          }
        } else if (target->type == OBJ_STRING) {
          String *str = (String *)target;
          if (i >= 0 && (uint64_t)i < str->len && val->type == OBJ_STRING && ((String *)val)->len == 1) { str->value[i] = ((String *)val)->value[0]; freeObject(val); }
          else { 
            if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range or invalid value.", vm->sourcetext);
            freeObject(val); freeObject(idx); freeObject(target); HANDLE_ERROR(); 
          }
        } else { 
          if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Target is not indexable.", vm->sourcetext);
          freeObject(val); freeObject(idx); freeObject(target); HANDLE_ERROR(); 
        }
        push(vm, (Object *)initInt(1)); freeObject(idx); freeObject(target); break;
      }
      case OP_STORE_INDEX: {
        String *name = (String *)READ_CONST(vm);
        Object *val = pop(vm); Object *idx = pop(vm);
        Object *target = getTable(vm->frames[vm->frameTop - 1].variables, name->value);
        if (!target) {
           char buf[256]; snprintf(buf, sizeof(buf), "Undefined variable \"%s\".", name->value);
           if (!*vm->err) *vm->err = initNameError((Position){0,0,0}, (Position){0,0,0}, vm->filename, buf, vm->sourcetext);
           freeObject(val); freeObject(idx); HANDLE_ERROR();
        }
        if (idx->type != OBJ_NUMBER_INT) {
          if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index must be an integer.", vm->sourcetext);
          freeObject(val); freeObject(idx); HANDLE_ERROR();
        }
        int64_t i = ((Number *)idx)->as.i;
        if (target->type == OBJ_LIST) {
          List *list = (List *)target;
          if (i >= 0 && (uint64_t)i < list->size) { freeObject(list->objects[i]); list->objects[i] = copyObject(val); }
          else {
            if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range.", vm->sourcetext);
            freeObject(val); freeObject(idx); HANDLE_ERROR();
          }
        } else if (target->type == OBJ_STRING) {
          String *str = (String *)target;
          if (i >= 0 && (uint64_t)i < str->len && val->type == OBJ_STRING && ((String *)val)->len == 1) { str->value[i] = ((String *)val)->value[0]; }
          else {
            if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range or invalid value.", vm->sourcetext);
            freeObject(val); freeObject(idx); HANDLE_ERROR();
          }
        } else {
          if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Target is not indexable.", vm->sourcetext);
          freeObject(val); freeObject(idx); HANDLE_ERROR();
        }
        push(vm, (Object *)initInt(1)); freeObject(val); freeObject(idx); break;
      }
      case OP_CALL: {
        uint8_t argCount = READ_BYTE(vm);
        Object *args[argCount];
        for (int i = argCount - 1; i >= 0; i--) args[i] = pop(vm);
        Object *callee = pop(vm);
        if (callee->type == OBJ_FUNCTION) {
          Function *func = (Function *)callee;
          SymbolTable *env = createTable(16, vm->frames[vm->frameTop - 1].variables);
          for (int i = 0; i < argCount; i++) {
            if (i < (int)func->paramCount) setTable(env, func->params[i], args[i], false);
            else freeObject(args[i]);
          }
          
          if (!func->chunk && func->body) {
             func->chunk = compileAST(func->body, vm->err, vm->filename, vm->sourcetext);
          }

          if (func->chunk) {
            if (vm->frameTop >= VM_CALL_STACK_MAX) {
               if (!*vm->err) *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Call stack overflow.", vm->sourcetext);
               freeObject(callee); HANDLE_ERROR();
            }
            vm->frames[vm->frameTop++] = (CallFrame){ .chunk = func->chunk, .ip = func->chunk->code, .variables = env, .tryStackTop = vm->tryStackTop };
            freeObject(callee);
            continue; // Will start executing the new frame in next iteration
          }
        } else if (callee->type == OBJ_NATIVE_FUNCTION) {
          NativeFunction *nf = (NativeFunction *)callee;
          Object *res = nf->function(args, argCount);
          for (int i = 0; i < argCount; i++) freeObject(args[i]);
          
          if (res) {
             if (res->type == OBJ_ERROR) {
               if (!*vm->err) *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, ((ProgramError*)res)->details, vm->sourcetext);
               freeObject(res); res = NULL;
             }
             if (res) push(vm, res);
          }
          if (!res) { freeObject(callee); HANDLE_ERROR(); }
        } else {
          freeObject(callee);
          HANDLE_ERROR();
        }
        freeObject(callee);
        break;
      }
      case OP_TRY_PUSH: {
        int16_t offset = READ_SHORT(vm);
        if (vm->tryStackTop < VM_TRY_STACK_MAX) vm->tryStack[vm->tryStackTop++] = vm->frames[vm->frameTop - 1].ip + offset;
        break;
      }
      case OP_TRY_POP: if (vm->tryStackTop > 0) vm->tryStackTop--; break;
      case OP_IMPORT: {
        String *pathObj = (String *)READ_CONST(vm); char *name = pathObj->value;
        bool found = false;
        for (size_t i = 0; stdlibModules[i]; i++) {
          if (strcmp(stdlibModules[i]->name, name) == 0) {
            stdlibModules[i]->init(vm->frames[vm->frameTop - 1].variables);
            push(vm, (Object *)initInt(1)); found = true; break;
          }
        }
        if (found) break;
        char *resolvedPath = resolveImportPath(vm->filename, name);
        char *fileContent = readFile(resolvedPath);
        if (!fileContent) { 
          if (!*vm->err) *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Failed to load imported file", vm->sourcetext);
          HANDLE_ERROR(); 
        }
        Lexer *lexer = initLexer(stringDup(name), fileContent);
        size_t tokenAmount = 0; Token *tokens = makeTokensLexer(lexer, vm->err, &tokenAmount);
        if (!tokens) { HANDLE_ERROR(); }
        Parser *parser = initParser(tokens, tokenAmount, vm->err, fileContent, name);
        ASTNode *ast = parseProgram(parser);
        if (!ast) { HANDLE_ERROR(); }
        Chunk *chunk = compileAST(ast, vm->err, name, fileContent);
        if (!chunk) { HANDLE_ERROR(); }
        if (vm->frameTop >= VM_CALL_STACK_MAX) {
           if (!*vm->err) *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Call stack overflow.", vm->sourcetext);
           HANDLE_ERROR();
        }
        vm->frames[vm->frameTop++] = (CallFrame){ .chunk = chunk, .ip = chunk->code, .variables = vm->frames[vm->frameTop - 2].variables, .tryStackTop = vm->tryStackTop };
        break;
      }
      case OP_RETURN: {
        Object *val = pop(vm);
        if (vm->frameTop > 1) {
          CallFrame *leavingFrame = &vm->frames[--vm->frameTop];
          if (leavingFrame->variables != vm->frames[vm->frameTop - 1].variables) {
             freeTable(leavingFrame->variables);
          }
          vm->tryStackTop = leavingFrame->tryStackTop;
          push(vm, val);
          continue; // Continue execution in previous frame
        }
        return val;
      }
      case OP_BREAK: return (Object *)initBreak();
      case OP_CONTINUE: return (Object *)initContinue();
      case OP_HALT: {
        Object *res = vm->stackTop > 0 ? pop(vm) : (Object *)initInt(0);
        if (vm->frameTop > 1) {
          CallFrame *leavingFrame = &vm->frames[--vm->frameTop];
          if (leavingFrame->variables != vm->frames[vm->frameTop - 1].variables) {
             freeTable(leavingFrame->variables);
          }
          vm->tryStackTop = leavingFrame->tryStackTop;
          push(vm, res);
          continue;
        }
        return res;
      }
    }
  }
}

VM *initVM(Chunk *chunk, SymbolTable *variables, Error **err, char *filename, char *sourcetext) {
  VM *vm = arenaNew(objectArena, VM);
  if (!vm) return NULL;
  vm->frameTop = 0;
  vm->frames[vm->frameTop++] = (CallFrame){ .chunk = chunk, .ip = chunk->code, .variables = variables, .tryStackTop = 0 };
  vm->stackTop = 0; vm->tryStackTop = 0;
  vm->err = err; vm->filename = filename; vm->sourcetext = sourcetext;
  return vm;
}
