#include "../include/vm.h"
#include "../include/object.h"
#include "../include/memarena.h"
#include "../include/utils.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/compiler.h"
#include "../include/node.h"
#include "../include/repl/readfile.h"
#include "../include/repl/repl.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOAD_STATE() \
  CallFrame *frame = &vm->frames[vm->frameTop - 1]; \
  uint8_t *ip = frame->ip; \
  SymbolTable* vars = frame->variables; \
  register Object **sp = vm->stack + vm->stackTop;

#define REFRESH_FRAME() do { \
  frame = &vm->frames[vm->frameTop - 1]; \
  ip = frame->ip; \
  vars = frame->variables; \
  sp = vm->stack + vm->stackTop; \
} while (0)

#define SAVE_STATE() do { \
  vm->stackTop = sp - vm->stack; \
  frame->ip = ip; \
} while (0)

#define PUSH(v) (*sp++ = (v))
#define POP() (*--sp)
#define TOP() (*(sp - 1))
#define PEEK(i) (*(sp - 1 - (i)))

#define READ_BYTE()  (*ip++)
#define READ_CONST() (frame->chunk->constants[READ_BYTE()])
#define READ_SHORT() (ip += 2, (int16_t)((ip[-2] << 8) | ip[-1]))

#define DISPATCH() do { op = READ_BYTE(); goto *dispatch[op]; } while(0)

static Object *doArith(VM *vm, OpCode op, Object *a, Object *b) {
  (void)vm;

  ObjType at = a->type;
  ObjType bt = b->type;

  bool aInt = at == OBJ_NUMBER_INT;
  bool bInt = bt == OBJ_NUMBER_INT;
  
  if (aInt && bInt) {
    if (_DEBUG) printf("[debug] Binary operation fast path (int && int)\n");

    Number* na = (Number*)a;
    Number* nb = (Number*)b;

    Number* dest;

    if (!nb->isStatic) {
      dest = nb;
    } else {
      dest = (Number*)copyObject(b);
    }
    
    switch (op) {
      case OP_ADD: 
        dest->as.i = na->as.i + dest->as.i; break;
      case OP_SUB: 
        dest->as.i = na->as.i - dest->as.i; break;
      case OP_MUL: 
        dest->as.i = na->as.i * dest->as.i; break;
      case OP_DIV: 
        if (dest->as.i == 0) {
          *vm->err = initValueError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Division by zero.", vm->sourcetext);
          freeObject((Object*)dest);
          freeObject(a);
          return NULL;
        }

        dest->as.i = na->as.i / dest->as.i; break;
      case OP_POW: 
        dest->as.i = pow(na->as.i, dest->as.i); break;
      case OP_EQ: 
        dest->as.i = na->as.i == dest->as.i; break;
      case OP_NE: 
        dest->as.i = na->as.i != dest->as.i; break;
      case OP_LT: 
        dest->as.i = na->as.i < dest->as.i; break;
      case OP_GT: 
        dest->as.i = na->as.i > dest->as.i; break;
      case OP_LTE: 
        dest->as.i = na->as.i <= dest->as.i; break;
      case OP_GTE: 
        dest->as.i = na->as.i >= dest->as.i; break;
      case OP_AND: 
        dest->as.i = na->as.i && dest->as.i; break;
      case OP_OR: 
        dest->as.i = na->as.i || dest->as.i; break;
      default:
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Incompatible types for operation.", vm->sourcetext); 
        freeObject((Object*)dest);
        freeObject(a);
        return NULL;
    }

    freeObject(a);

    return (Object*)dest;
  }

  bool aNum = aInt || at == OBJ_NUMBER_FLOAT;
  bool bNum = bInt || bt == OBJ_NUMBER_FLOAT;

  if (at == OBJ_STRING || bt == OBJ_STRING) {
    Object* res = NULL;

    if (op == OP_ADD && at == OBJ_STRING && bt == OBJ_STRING) 
      res = (Object *)addString((String *)a, (String *)b);
    else if (op == OP_MUL && at == OBJ_STRING && bt == OBJ_NUMBER_INT) 
      res = (Object *)mulString((String *)a, (Number *)b);
    else if (op == OP_EQ) 
      res = (Object *)initInt(at == OBJ_STRING && bt == OBJ_STRING && strcmp(((String *)a)->value, ((String *)b)->value) == 0);
    else if (op == OP_NE)
      res = (Object *)initInt(!(at == OBJ_STRING && bt == OBJ_STRING && strcmp(((String *)a)->value, ((String *)b)->value) == 0));
    
    freeObject(a);
    freeObject(b);

    return res;
  }

  if (!aNum || !bNum) {
    freeObject(a);
    freeObject(b);

    return NULL;
  }

  Number* na = (Number*)a;
  Number* nb = (Number*)b;

  Number* dest;

  if (!nb->isStatic) {
    dest = nb;
  } else {
    dest = (Number*)copyObject(b);
  }  

  ErrType result = ERR_NONE;

  switch (op) {
    case OP_ADD: result = addNumber(dest, na); break;
    case OP_SUB: result = subNumber(dest, na); break;
    case OP_MUL: result = mulNumber(dest, na); break;
    case OP_DIV: result = divNumber(dest, na); break;
    case OP_POW: result = powNumber(dest, na); break;
    case OP_EQ: result = isEqualNumber(dest, na); break;
    case OP_NE: result = isNotEqualNumber(dest, na); break;
    case OP_LT: result = isLessThanNumber(dest, na); break;
    case OP_GT: result = isGreaterThanNumber(dest, na); break;
    case OP_LTE: result = isLessThanEqualNumber(dest, na); break;
    case OP_GTE: result = isGreaterThanEqualNumber(dest, na); break;
    case OP_AND: result = andNumber(dest, na); break;
    case OP_OR: result = orNumber(dest, na); break;
    default: result = ERR_TYPE;
  }

  freeObject(a);

  if (result != ERR_NONE) {
    if (!*vm->err) {
      if (result == ERR_DIV_BY_ZERO)
        *vm->err = initValueError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Division by zero.", vm->sourcetext);
      else if (result == ERR_TYPE)
        *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Incompatible types for operation.", vm->sourcetext);
      else if (result == ERR_NULL)
        *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Null operand in arithmetic operation.", vm->sourcetext);
    }

    freeObject((Object*)dest);

    return NULL;
  }

  return (Object *)dest;
}

#define HANDLE_ERROR() \
  if (vm->tryStackTop > 0) { \
    ip = vm->tryStack[--vm->tryStackTop]; \
    if (*vm->err) { \
      Object *errStr = (Object *)initString((*vm->err)->details, strlen((*vm->err)->details)); \
      PUSH(errStr); \
      freeError(*vm->err); \
      *vm->err = NULL; \
    } else { \
      PUSH((Object *)initString("Unknown Error", 13)); \
    } \
    SAVE_STATE(); \
    continue; \
  } else { \
    SAVE_STATE(); \
    return NULL; \
  }

Object *vmRun(VM *vm) {
  OpCode op;
  
  LOAD_STATE();

  static void *dispatch[] = {
    [OP_LOAD_CONST] = &&OP_LOAD_CONST,
    [OP_LOAD_VAR] = &&OP_LOAD_VAR,
    [OP_STORE_VAR] = &&OP_STORE_VAR,

    [OP_POP] = &&OP_POP,

    [OP_ADD] = &&OP_ADD,
    [OP_SUB] = &&OP_SUB,
    [OP_MUL] = &&OP_MUL,
    [OP_DIV] = &&OP_DIV,
    [OP_POW] = &&OP_POW,

    [OP_EQ] = &&OP_EQ,
    [OP_NE] = &&OP_NE,
    [OP_LT] = &&OP_LT,
    [OP_GT] = &&OP_GT,
    [OP_LTE] = &&OP_LTE,
    [OP_GTE] = &&OP_GTE,

    [OP_AND] = &&OP_AND,
    [OP_OR] = &&OP_OR,

    [OP_NEG] = &&OP_NEG,
    [OP_NOT] = &&OP_NOT,

    [OP_JUMP] = &&OP_JUMP,
    [OP_JUMP_IF_FALSE] = &&OP_JUMP_IF_FALSE,

    [OP_FOR_PREP] = &&OP_FOR_PREP,
    [OP_FOR_ITER] = &&OP_FOR_ITER,

    [OP_BUILD_LIST] = &&OP_BUILD_LIST,

    [OP_INDEX_GET] = &&OP_INDEX_GET,
    [OP_INDEX_SET] = &&OP_INDEX_SET,
    [OP_STORE_INDEX] = &&OP_STORE_INDEX,

    [OP_CALL] = &&OP_CALL,

    [OP_TRY_PUSH] = &&OP_TRY_PUSH,
    [OP_TRY_POP] = &&OP_TRY_POP,

    [OP_IMPORT] = &&OP_IMPORT,

    [OP_RETURN] = &&OP_RETURN,
    [OP_BREAK] = &&OP_BREAK,
    [OP_CONTINUE] = &&OP_CONTINUE,
    [OP_HALT] = &&OP_HALT,
  };

  for (;;) {
    DISPATCH();

    OP_LOAD_CONST: 
      PUSH(copyObject(READ_CONST()));
      DISPATCH();

    OP_LOAD_VAR: {
      String *name = (String *)READ_CONST();
      Object *val = getTable(vars, name->value);

      if (!val) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Undefined variable \"%s\".", name->value);
        if (!*vm->err) *vm->err = initNameError((Position){0,0,0}, (Position){0,0,0}, vm->filename, buf, vm->sourcetext);
        HANDLE_ERROR();
      }

      PUSH(copyObject(val));
      DISPATCH();
    }
      
    OP_STORE_VAR: {
      String *name = (String *)READ_CONST();
      Object *val  = PEEK(0);
      setTable(vars, name->value, val, true);
      DISPATCH();
    }

    OP_POP: { 
      Object *o = POP();
      if (o) freeObject(o);
      DISPATCH();
    }

    OP_ADD: OP_SUB: OP_MUL: OP_DIV:
    OP_POW: OP_EQ:  OP_NE:  OP_LT:
    OP_GT:  OP_LTE: OP_GTE: OP_AND: OP_OR: {
      Object *b = POP();
      Object *a = POP();

      Object *res = doArith(vm, op, a, b);

      if (!res) {
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Incompatible types for operation", vm->sourcetext);
        HANDLE_ERROR();
      }

      PUSH(res);
      DISPATCH();
    }

    OP_NEG: {
      Object *a = POP();

      if (a->type == OBJ_NUMBER_INT) 
        PUSH((Object *)initInt(-((Number *)a)->as.i));
      else if (a->type == OBJ_NUMBER_FLOAT) 
        PUSH((Object *)initFloat(-((Number *)a)->as.f));
      else { 
        freeObject(a); 
        HANDLE_ERROR();
      }

      freeObject(a); 
      DISPATCH();
    }

    OP_NOT: {
      Object *a = POP();

      if (a->type == OBJ_NUMBER_INT) 
        PUSH((Object *)initInt(!((Number *)a)->as.i));
      else if (a->type == OBJ_NUMBER_FLOAT) 
        PUSH((Object *)initFloat(!((Number *)a)->as.f));
      else { 
        freeObject(a);
        HANDLE_ERROR();
      }

      freeObject(a);
      DISPATCH();
    }

    OP_JUMP: { 
      int16_t offset = READ_SHORT();
      ip += offset;
      DISPATCH();
    }

    OP_JUMP_IF_FALSE: {
      int16_t offset = READ_SHORT();
      Object *cond = POP();

      if (cond->type != OBJ_NUMBER_INT) {
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Condition must be an integer.", vm->sourcetext);
        freeObject(cond);
        HANDLE_ERROR();
      }

      if (((Number *)cond)->as.i == 0)
        ip += offset;

      freeObject(cond);
      DISPATCH();
    }

    OP_FOR_PREP: {
      Object *iterable = POP();

      if (iterable->type != OBJ_LIST && iterable->type != OBJ_STRING) { 
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Object is not iterable", vm->sourcetext);
        freeObject(iterable); HANDLE_ERROR(); 
      }

      PUSH(iterable);
      PUSH((Object *)initInt(iterable->type == OBJ_LIST ? ((List *)iterable)->size : ((String *)iterable)->len));
      PUSH((Object *)initInt(0)); // index
      
      DISPATCH();
    }

    OP_FOR_ITER: {
      int16_t offset = READ_SHORT();
      Number *index = (Number *)PEEK(0);
      Number *length = (Number *)PEEK(1);
      Object *iterable = PEEK(2);

      if (index->as.i < length->as.i) {
        Object *item;

        if (iterable->type == OBJ_LIST) 
          item = copyObject(((List *)iterable)->objects[index->as.i]);
        else { 
          char buf[2] = { ((String *)iterable)->value[index->as.i], '\0' };
          item = (Object *)initString(buf, 1);
        }

        PUSH(item);
        index->as.i++;
      } else {
        ip += offset;
      }
      
      DISPATCH();
    }

    OP_BUILD_LIST: {
      uint8_t count = READ_BYTE();
      Object *items[256];

      for (int i = count - 1; i >= 0; i--) 
        items[i] = POP();

      PUSH((Object *)initList(items, count, count));
      DISPATCH();
    }

    OP_INDEX_GET: {
      Object *idx = POP();
      Object *target = POP();

      if (idx->type != OBJ_NUMBER_INT) { 
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index must be an integer.", vm->sourcetext);
        freeObject(idx);
        freeObject(target);
        HANDLE_ERROR(); 
      }

      int64_t i = ((Number *)idx)->as.i;

      if (target->type == OBJ_STRING) {
        String *str = (String *)target;

        if (i < 0 || (uint64_t)i >= str->len) { 
          if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range.", vm->sourcetext);
          freeObject(idx);
          freeObject(target);
          HANDLE_ERROR(); 
        }

        char buf[2] = { str->value[i], '\0' };
        PUSH((Object *)initString(buf, 1));

      } else if (target->type == OBJ_LIST) {
        List *list = (List *)target;

        if (i < 0 || (uint64_t)i >= list->size) { 
          if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range.", vm->sourcetext);
          freeObject(idx);
          freeObject(target);
          HANDLE_ERROR(); 
        }

        PUSH(copyObject(list->objects[i]));

      } else { 
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Target is not indexable.", vm->sourcetext);
        freeObject(idx);
        freeObject(target);
        HANDLE_ERROR(); 
      }
      freeObject(idx);
      freeObject(target);
      DISPATCH();
    }

    OP_INDEX_SET: {
      Object *val = POP();
      Object *idx = POP();
      Object *target = POP();

      if (idx->type != OBJ_NUMBER_INT) { 
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index must be an integer.", vm->sourcetext);
        freeObject(val);
        freeObject(idx);
        freeObject(target);
        HANDLE_ERROR(); 
      }

      int64_t i = ((Number *)idx)->as.i;

      if (target->type == OBJ_LIST) {
        List *list = (List *)target;

        if (i >= 0 && (uint64_t)i < list->size) { 
          freeObject(list->objects[i]);
          list->objects[i] = val; 
        }

        else { 
          if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range.", vm->sourcetext);
          freeObject(val);
          freeObject(idx);
          freeObject(target);
          HANDLE_ERROR(); 
        }

      } else if (target->type == OBJ_STRING) {
        String *str = (String *)target;

        if (i >= 0 && (uint64_t)i < str->len && val->type == OBJ_STRING && ((String *)val)->len == 1) { 
          str->value[i] = ((String *)val)->value[0]; 
          freeObject(val);
        }

        else { 
          if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range or invalid value.", vm->sourcetext);
          freeObject(val);
          freeObject(idx);
          freeObject(target);
          HANDLE_ERROR(); 
        }
      } else { 
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Target is not indexable.", vm->sourcetext);
        freeObject(val);
        freeObject(idx);
        freeObject(target);
        HANDLE_ERROR(); 
      }

      PUSH((Object *)initInt(1));
      freeObject(idx);
      freeObject(target);
      
      DISPATCH();
    }

    OP_STORE_INDEX: {
      String *name = (String *)READ_CONST();
      Object *val = POP();
      Object *idx = POP();
      Object *target = getTable(vars, name->value);

      if (!target) {
        char buf[256]; snprintf(buf, sizeof(buf), "Undefined variable \"%s\".", name->value);
        if (!*vm->err) *vm->err = initNameError((Position){0,0,0}, (Position){0,0,0}, vm->filename, buf, vm->sourcetext);
        freeObject(val);
        freeObject(idx);
        HANDLE_ERROR();
      }

      if (idx->type != OBJ_NUMBER_INT) {
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index must be an integer.", vm->sourcetext);
        freeObject(val);
        freeObject(idx);
        HANDLE_ERROR();
      }

      int64_t i = ((Number *)idx)->as.i;

      if (target->type == OBJ_LIST) {
        List *list = (List *)target;

        if (i >= 0 && (uint64_t)i < list->size) { 
          freeObject(list->objects[i]);
          list->objects[i] = copyObject(val);
        } else {
          if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range.", vm->sourcetext);
          freeObject(val);
          freeObject(idx);
          HANDLE_ERROR();
        }

      } else if (target->type == OBJ_STRING) {
        String *str = (String *)target;

        if (i >= 0 && (uint64_t)i < str->len && val->type == OBJ_STRING && ((String *)val)->len == 1) { 
          str->value[i] = ((String *)val)->value[0];
        } else {
          if (!*vm->err) *vm->err = initIndexError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Index out of range or invalid value.", vm->sourcetext);
          freeObject(val);
          freeObject(idx);
          HANDLE_ERROR();
        }
      } else {
        if (!*vm->err) *vm->err = initTypeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Target is not indexable.", vm->sourcetext);
        freeObject(val);
        freeObject(idx);
        HANDLE_ERROR();
      }

      PUSH((Object *)initInt(1));
      freeObject(val);
      freeObject(idx);
      DISPATCH();
    }

    OP_CALL: {
      uint8_t argCount = READ_BYTE();
      Object *args[256];

      for (int i = argCount - 1; i >= 0; i--) 
        args[i] = POP();

      Object *callee = POP();

      if (callee->type == OBJ_FUNCTION) {
        Function *func = (Function *)callee;
        SymbolTable *env = createTable(16, vars);

        for (int i = 0; i < argCount; i++) {
          if (i < (int)func->paramCount) 
            setTable(env, func->params[i], args[i], false);
          else 
            freeObject(args[i]);
        }
          
        if (!func->chunk && func->body) {
          func->chunk = compileAST(func->body, vm->err, vm->filename, vm->sourcetext);
        }

        if (func->chunk) {
          if (vm->frameTop >= VM_CALL_STACK_MAX) {
            if (!*vm->err) *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Call stack overflow.", vm->sourcetext);
            freeObject(callee); HANDLE_ERROR();
          }
          
          SAVE_STATE();

          vm->frames[vm->frameTop++] = (CallFrame){ 
            .chunk = func->chunk,
            .ip = func->chunk->code,
            .variables = env,
            .tryStackTop = vm->tryStackTop
          };

          REFRESH_FRAME();
          freeObject(callee);
          DISPATCH(); // Will start executing the new frame in next iteration
        }
      } else if (callee->type == OBJ_NATIVE_FUNCTION) {
        NativeFunction *nf = (NativeFunction *)callee;
        Object *res = nf->function(args, argCount);

        for (int i = 0; i < argCount; i++) 
          freeObject(args[i]);
          
        if (res) {
          if (res->type == OBJ_ERROR) {
            if (!*vm->err) *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, ((ProgramError*)res)->details, vm->sourcetext);
            freeObject(res); 
            res = NULL;
          }

          if (res) 
            PUSH(res);
        }

        if (!res) { 
          freeObject(callee);
          HANDLE_ERROR();
        }
      } else {
        freeObject(callee);
        HANDLE_ERROR();
      }

      freeObject(callee);
      DISPATCH();
    }

    OP_TRY_PUSH: {
      int16_t offset = READ_SHORT();

      if (vm->tryStackTop < VM_TRY_STACK_MAX) 
        vm->tryStack[vm->tryStackTop++] = ip + offset;

      DISPATCH();
    }

    OP_TRY_POP:
      if (vm->tryStackTop > 0)
      vm->tryStackTop--;
      DISPATCH();

    OP_IMPORT: {
      String *pathObj = (String *)READ_CONST();
      char *name = pathObj->value;
      bool found = false;

      for (size_t i = 0; stdlibModules[i]; i++) {
        if (strcmp(stdlibModules[i]->name, name) == 0) {
          stdlibModules[i]->init(vars);
          PUSH((Object *)initInt(1));
          found = true;
          break;
        }
      }

      if (found) DISPATCH();

      char *resolvedPath = resolveImportPath(vm->filename, name);
      char *fileContent = readFile(resolvedPath);

      if (!fileContent) { 
        if (!*vm->err) *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Failed to load imported file", vm->sourcetext);
        HANDLE_ERROR(); 
      }

      Lexer *lexer = initLexer(stringDup(name), fileContent);
      size_t tokenAmount = 0; Token *tokens = makeTokensLexer(lexer, vm->err, &tokenAmount);

      if (!tokens) { 
        HANDLE_ERROR();
      }

      Parser *parser = initParser(tokens, tokenAmount, vm->err, fileContent, name);
      ASTNode *ast = parseProgram(parser);

      if (!ast) { 
        HANDLE_ERROR();
      }

      Chunk *chunk = compileAST(ast, vm->err, name, fileContent);

      if (!chunk) { 
        HANDLE_ERROR();
      }

      if (vm->frameTop >= VM_CALL_STACK_MAX) {
        if (!*vm->err) *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Call stack overflow.", vm->sourcetext);
        HANDLE_ERROR();
      }

      SAVE_STATE();

      vm->frames[vm->frameTop++] = (CallFrame){ 
        .chunk = chunk,
        .ip = chunk->code,
        .variables = vars,
        .tryStackTop = vm->tryStackTop
      };

      REFRESH_FRAME();

      DISPATCH();
    }

    OP_RETURN: {
      Object *val = POP();

      if (vm->frameTop > 1) {
        CallFrame *leavingFrame = &vm->frames[--vm->frameTop];

        if (leavingFrame->variables != vm->frames[vm->frameTop-1].variables) {
            freeTable(leavingFrame->variables);
        }

        vm->tryStackTop = leavingFrame->tryStackTop;
        
        SAVE_STATE();
        REFRESH_FRAME();
        PUSH(val);

        DISPATCH(); 
      }
      
      SAVE_STATE();
      return val;
    }

    OP_BREAK: 
      return (Object *)initBreak();
    OP_CONTINUE: 
      return (Object *)initContinue();

    OP_HALT: {
      Object *res = (sp > vm->stack) ? POP() : (Object *)initInt(0);

      if (vm->frameTop > 1) {
        CallFrame *leavingFrame = &vm->frames[--vm->frameTop];

        if (leavingFrame->variables != vm->frames[vm->frameTop - 1].variables) {
            freeTable(leavingFrame->variables);
        }

        vm->tryStackTop = leavingFrame->tryStackTop;

        SAVE_STATE();
        REFRESH_FRAME();
        PUSH(res);
        
        DISPATCH();
      }

      SAVE_STATE();
      return res;
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
