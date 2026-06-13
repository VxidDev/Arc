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
  register Value *sp = vm->stack + vm->stackTop;

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

#define LOCAL(slot) (vm->locals[frame->localsBase + (slot)])

#define DISPATCH() do { \
  op = READ_BYTE(); \
  goto *dispatch[op]; \
} while(0)

#define VM_ERR(initFn, msg) do { \
  if (!*vm->err) { \
    frame->currentInstr = (uint32_t)(frame->ip - frame->chunk->code); \
    Position _ps, _pe; \
    vmGetPos(frame, &_ps, &_pe); \
    *vm->err = initFn(_ps, _pe, frame->chunk->filename, msg, frame->chunk->sourcetext); \
  } \
} while(0)

static inline void freeValue(Value v) {
  if (IS_OBJ(v)) freeObject(AS_OBJ(v));
}

static inline Value copyValue(Value v) {
  if (!IS_OBJ(v)) return v;

  Object* obj = AS_OBJ(v);
  if (obj->isStatic) return v;

  return VAL_OBJ(copyObject(obj));
}

static inline Value objectToValue(Object *o) {
  if (!o) return VAL_INT(0);
  if (o->type == OBJ_NUMBER_INT) return VAL_INT(((Number*)o)->as.i);
  if (o->type == OBJ_NUMBER_FLOAT) return VAL_FLOAT(((Number*)o)->as.f);
  return VAL_OBJ(o);
}

static inline Object *valueToObject(Value v) {
  if (IS_INT(v)) return (Object*)initInt(AS_INT(v));
  if (IS_FLOAT(v)) return (Object*)initFloat(AS_FLOAT(v));
  return AS_OBJ(v);
}

static void vmGetPos(CallFrame *frame, Position *start, Position *end) {
  Chunk *chunk = frame->chunk;
  PosEntry *entries = chunk->positions;
  size_t count = chunk->posCount;

  if (count == 0) {
    *start = (Position){0,0,0};
    *end = (Position){0,0,0};
    return;
  }

  size_t lo = 0, hi = count;

  while (lo < hi) {
    size_t mid = (lo + hi) >> 1;
    if (entries[mid].offset <= frame->currentInstr) lo = mid + 1;
    else hi = mid;
  }

  size_t idx = (lo == 0) ? 0 : lo - 1;
  *start = entries[idx].start;
  *end = entries[idx].end;
}

static Value doArith(VM *vm, CallFrame* frame, OpCode op, Value a, Value b) {
  (void)vm; 

  if ((IS_INT(a) || IS_FLOAT(a)) && (IS_INT(b) || IS_FLOAT(b))) {
    double na = IS_INT(a) ? (double)AS_INT(a) : AS_FLOAT(a);
    double nb = IS_INT(b) ? (double)AS_INT(b) : AS_FLOAT(b);

    switch (op) {
      case OP_ADD: return VAL_FLOAT(na + nb);
      case OP_SUB: return VAL_FLOAT(na - nb);
      case OP_MUL: return VAL_FLOAT(na * nb);
      case OP_DIV: 
        if (nb == 0.0) {
          VM_ERR(initValueError, "Division by zero.");
          return VAL_INT(0);
        }
        return VAL_FLOAT(na / nb);
      case OP_POW: return VAL_FLOAT(pow(na, nb));
      case OP_EQ: return VAL_INT(na == nb); 
      case OP_NE: return VAL_INT(na != nb); 
      case OP_LT: return VAL_INT(na < nb); 
      case OP_GT: return VAL_INT(na > nb); 
      case OP_LTE: return VAL_INT(na <= nb); 
      case OP_GTE: return VAL_INT(na >= nb); 
      case OP_AND: return VAL_INT(na && nb); 
      case OP_OR: return VAL_INT(na || nb); 
      default: break;
    }
  }

  // Handle strings
  Object* aObj = valueToObject(a);
  Object* bObj = valueToObject(b);

  bool aStr = aObj && aObj->type == OBJ_STRING;
  bool bStr = bObj && bObj->type == OBJ_STRING;

  if (aStr || bStr) {
    Value res = VAL_INT(0);
    if (op == OP_ADD && aStr && bStr) 
      res = VAL_OBJ((Object *)addString((String *)aObj, (String *)bObj));
    else if (op == OP_MUL && aStr && (IS_INT(b) || IS_FLOAT(b))) {
      Number* n = (Number*)bObj;
      res = VAL_OBJ((Object *)mulString((String *)aObj, n));
    }
    else if (op == OP_EQ) 
      res = VAL_INT(aStr && bStr && strcmp(((String *)aObj)->value, ((String *)bObj)->value) == 0);
    else if (op == OP_NE)
      res = VAL_INT(!(aStr && bStr && strcmp(((String *)aObj)->value, ((String *)bObj)->value) == 0));
    
    freeValue(a);
    freeValue(b);
    return res;
  }

  // Fallback to Number objects (should be rare now)
  if (aObj->type == OBJ_NUMBER_INT || aObj->type == OBJ_NUMBER_FLOAT) {
    Number* na = (Number*)aObj;
    Number* nb = (Number*)bObj;
    ErrType result = ERR_NONE;

    switch (op) {
      case OP_ADD: result = addNumber(nb, na); break;
      case OP_SUB: result = subNumber(nb, na); break;
      case OP_MUL: result = mulNumber(nb, na); break;
      case OP_DIV: result = divNumber(nb, na); break;
      case OP_POW: result = powNumber(nb, na); break;
      case OP_EQ: result = isEqualNumber(nb, na); break;
      case OP_NE: result = isNotEqualNumber(nb, na); break;
      case OP_LT: result = isLessThanNumber(nb, na); break;
      case OP_GT: result = isGreaterThanNumber(nb, na); break;
      case OP_LTE: result = isLessThanEqualNumber(nb, na); break;
      case OP_GTE: result = isGreaterThanEqualNumber(nb, na); break;
      case OP_AND: result = andNumber(nb, na); break;
      case OP_OR: result = orNumber(nb, na); break;
      default: result = ERR_TYPE;
    }

    freeValue(a);
    if (result != ERR_NONE) {
      if (!*vm->err) {
        if (result == ERR_DIV_BY_ZERO) VM_ERR(initValueError, "Division by zero.");
        else VM_ERR(initTypeError, "Incompatible types for operation.");
      }
      freeValue(b);
      return VAL_INT(0);
    }
    Value final = objectToValue((Object*)nb);
    return final;
  }

  VM_ERR(initTypeError, "Incompatible types for operation.");
  freeValue(a);
  freeValue(b);
  return VAL_INT(0);
}

#define HANDLE_ERROR() \
  if (vm->tryStackTop > 0) { \
    ip = vm->tryStack[--vm->tryStackTop]; \
    if (*vm->err) { \
      Object *errStr = (Object *)initString((*vm->err)->details, strlen((*vm->err)->details)); \
      PUSH(objectToValue(errStr)); \
      freeError(*vm->err); \
      *vm->err = NULL; \
    } else { \
      PUSH(objectToValue((Object *)initString("Unknown Error", 13))); \
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

    [OP_LOAD_LOCAL] = &&OP_LOAD_LOCAL,
    [OP_STORE_LOCAL] = &&OP_STORE_LOCAL
  };

  for (;;) {
    DISPATCH();

    OP_LOAD_CONST: {
      Object *c = READ_CONST();

      if (c->type == OBJ_NUMBER_INT)
        PUSH(VAL_INT(((Number*)c)->as.i));
      else if (c->type == OBJ_NUMBER_FLOAT)
        PUSH(VAL_FLOAT(((Number*)c)->as.f));
      else if (c->type == OBJ_FUNCTION || c->type == OBJ_STRING)
        PUSH(VAL_OBJ(c)); // constants are owned by chunk
      else if (c->isStatic)
        PUSH(VAL_OBJ(c));
      else
        PUSH(copyValue(objectToValue(c)));

      DISPATCH();
    } 

    OP_LOAD_VAR: {
      String *name = (String *)READ_CONST();
      Object* val = getTable(vars, name->value);

      if (!val) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Undefined variable \"%s\".", name->value);
        VM_ERR(initNameError, buf); 
        HANDLE_ERROR();
      }

      if (val->type == OBJ_NUMBER_INT)
        PUSH(VAL_INT(((Number*)val)->as.i));
      else if (val->type == OBJ_NUMBER_FLOAT)
        PUSH(VAL_FLOAT(((Number*)val)->as.f));
      else
        PUSH(copyValue(objectToValue(val)));

      DISPATCH();
    }
      
    OP_STORE_VAR: {
      String *name = (String *)READ_CONST();
      Value val = PEEK(0);

      if (IS_INT(val)) {
        setTable(vars, name->value, (Object*)initInt(AS_INT(val)), false);
      } else if (IS_FLOAT(val)) {
        setTable(vars, name->value, (Object*)initFloat(AS_FLOAT(val)), false);
      } else {
        setTable(vars, name->value, AS_OBJ(val), true);
      }

      DISPATCH();
    }

    OP_LOAD_LOCAL: {
      uint8_t slot = READ_BYTE();
      Value val = LOCAL(slot);

      if (IS_UNDEF(val)) {
        VM_ERR(initNameError, "Variable used before assignment.");
        HANDLE_ERROR();
      }

      if (IS_OBJ(val) && AS_OBJ(val)->isStatic) 
        PUSH(val);
      else 
        PUSH(IS_OBJ(val) ? copyValue(val) : val);
      
      DISPATCH();
    }

    OP_STORE_LOCAL: {
      uint8_t slot = READ_BYTE();
      Value val = PEEK(0);

      if (IS_OBJ(val)) {
        Object* o = AS_OBJ(val);

        if (o->type == OBJ_NUMBER_INT)
          val = VAL_INT(((Number*)o)->as.i);
        else if (o->type == OBJ_NUMBER_FLOAT)
          val = VAL_FLOAT(((Number*)o)->as.f);
      }

      freeValue(LOCAL(slot));
      LOCAL(slot) = IS_OBJ(val) ? copyValue(val) : val;

      DISPATCH();
    } 

    OP_POP: { 
      Value val = POP();
      freeValue(val);
      DISPATCH();
    }

    OP_ADD: OP_SUB: OP_MUL: OP_DIV:
    OP_POW: OP_EQ:  OP_NE:  OP_LT:
    OP_GT: OP_LTE: OP_GTE: OP_AND: OP_OR: {
      Value b = POP();
      Value a = POP();

      if (IS_INT(a) && IS_INT(b)) {
        int64_t na = AS_INT(a);
        int64_t nb = AS_INT(b);

        switch (op) {
          case OP_ADD: PUSH(VAL_INT(na + nb)); DISPATCH();
          case OP_SUB: PUSH(VAL_INT(na - nb)); DISPATCH();
          case OP_MUL: PUSH(VAL_INT(na * nb)); DISPATCH();
          case OP_DIV:
            if (nb == 0) { VM_ERR(initValueError, "Division by zero."); HANDLE_ERROR(); }
            PUSH(VAL_FLOAT((double)na / nb)); DISPATCH();
          case OP_POW: PUSH(VAL_FLOAT(pow((double)na, (double)nb))); DISPATCH();
          case OP_EQ:  PUSH(VAL_INT(na == nb)); DISPATCH();
          case OP_NE:  PUSH(VAL_INT(na != nb)); DISPATCH();
          case OP_LT:  PUSH(VAL_INT(na < nb));  DISPATCH();
          case OP_GT:  PUSH(VAL_INT(na > nb));  DISPATCH();
          case OP_LTE: PUSH(VAL_INT(na <= nb)); DISPATCH();
          case OP_GTE: PUSH(VAL_INT(na >= nb)); DISPATCH();
          case OP_AND: PUSH(VAL_INT(na && nb)); DISPATCH();
          case OP_OR:  PUSH(VAL_INT(na || nb)); DISPATCH();
          default: break;
        }
      }

      Value res = doArith(vm, frame, op, a, b);
      if (*vm->err) { freeValue(res); HANDLE_ERROR(); }
      PUSH(res);
      DISPATCH();
    }

    OP_NEG: {
      Value a = POP();

      if (a.type == VAL_INT) 
        PUSH(VAL_INT(-AS_INT(a)));
      else if (a.type == VAL_FLOAT) 
        PUSH(VAL_FLOAT(-AS_FLOAT(a)));
      else { 
        freeValue(a); 
        HANDLE_ERROR();
      }

      freeValue(a); 
      DISPATCH();
    }

    OP_NOT: {
      Value a = POP();

      if (a.type == VAL_INT) 
        PUSH(VAL_INT(!AS_INT(a)));
      else if (a.type == VAL_FLOAT) 
        PUSH(VAL_INT(!AS_FLOAT(a)));
      else { 
        freeValue(a);
        HANDLE_ERROR();
      }

      freeValue(a);
      DISPATCH();
    }

    OP_JUMP: { 
      int16_t offset = READ_SHORT();
      ip += offset;
      DISPATCH();
    }

    OP_JUMP_IF_FALSE: {
      int16_t offset = READ_SHORT();
      Value cond = POP();

      if (cond.type != VAL_INT) {
        VM_ERR(initTypeError, "Condition must be an integer.");
        freeValue(cond);
        HANDLE_ERROR();
      }

      if (AS_INT(cond) == 0)
        ip += offset;

      freeValue(cond);
      DISPATCH();
    }

    OP_FOR_PREP: {
      Value iterVal = POP();

      if (!IS_OBJ(iterVal) || (AS_OBJ(iterVal)->type != OBJ_LIST && AS_OBJ(iterVal)->type != OBJ_STRING)) {
        VM_ERR(initTypeError, "Object is not iterable");
        freeValue(iterVal); HANDLE_ERROR(); 
      }

      Object* iterable = AS_OBJ(iterVal);
      PUSH(iterVal);
      PUSH(VAL_INT(iterable->type == OBJ_LIST ? ((List *)iterable)->size : ((String *)iterable)->len));
      PUSH(VAL_INT(0)); // index
      
      DISPATCH();
    }

    OP_FOR_ITER: {
      int16_t offset = READ_SHORT();
      Value indexVal = PEEK(0);
      Value lengthVal = PEEK(1);
      Value iterVal = PEEK(2);

      if (AS_INT(indexVal) < AS_INT(lengthVal)) {
        Object *iterable = AS_OBJ(iterVal);
        Value item;

        if (iterable->type == OBJ_LIST) {
          Object *elem = ((List *)iterable)->objects[AS_INT(indexVal)];

          if (elem->type == OBJ_NUMBER_INT)
            item = VAL_INT(((Number*)elem)->as.i);
          else if (elem->type == OBJ_NUMBER_FLOAT)
            item = VAL_FLOAT(((Number*)elem)->as.f);
          else
            item = copyValue(objectToValue(elem));
        } else { 
          char buf[2] = { ((String *)iterable)->value[AS_INT(indexVal)], '\0' };
          item = VAL_OBJ((Object *)initString(buf, 1));
        }

        PUSH(item);
        (sp - 2)->as.i++; 
      } else {
        ip += offset;
      }
      
      DISPATCH();
    }

    OP_BUILD_LIST: {
      uint8_t count = READ_BYTE();
      Object **items = malloc(sizeof(Object*) * count);

      // Stack: [..., item0, item1, ..., itemN] (itemN is at top/PEEK(0))
      // Items list should be [item0, item1, ..., itemN]
      for (int i = 0; i < count; i++) 
        items[count - 1 - i] = valueToObject(PEEK(i));

      for (int i = 0; i < count; i++) {
          Value v = POP();
          freeValue(v);
      }

      PUSH(VAL_OBJ((Object*)initList(items, count, count)));
      free(items);
      DISPATCH();
    }

    OP_INDEX_GET: {
      Value idxVal = POP();
      Value targetVal = POP();

      if (!IS_INT(idxVal)) { 
        VM_ERR(initIndexError, "Index must be an integer."); 
        freeValue(idxVal);
        freeValue(targetVal);
        HANDLE_ERROR(); 
      }

      int64_t i = AS_INT(idxVal);
      Object *target = valueToObject(targetVal);

      // Ensure we don't free the target object itself if it's held by the list/variable
      // valueToObject creates a temporary Object* if idxVal is primitive.
      // If targetVal is OBJ_LIST, target is a pointer to it.
      
      if (target->type == OBJ_STRING) {
        String *str = (String *)target;

        if (i < 0 || (uint64_t)i >= str->len) { 
          VM_ERR(initIndexError, "Index out of range."); 
          freeValue(idxVal);
          freeValue(targetVal);
          HANDLE_ERROR(); 
        }

        char buf[2] = { str->value[i], '\0' };
        PUSH(VAL_OBJ((Object *)initString(buf, 1)));

      } else if (target->type == OBJ_LIST) {
        List *list = (List *)target;

        if (i < 0 || (uint64_t)i >= list->size) { 
          VM_ERR(initIndexError, "Index out of range.");
          freeValue(idxVal);
          freeValue(targetVal);
          HANDLE_ERROR(); 
        }

        PUSH(objectToValue(list->objects[i]));

      } else { 
        VM_ERR(initTypeError, "Target is not indexable.");
        freeValue(idxVal);
        freeValue(targetVal);
        HANDLE_ERROR(); 
      }
      
      // If targetVal was an object, do not free it here.
      // But if it was a primitive (e.g. initInt), it wasn't an OBJ_LIST/STRING.
      // The current logic seems to only reach here if it was OBJ_LIST/STRING.
      // valueToObject does NOT take ownership, so this is fine.
      freeValue(idxVal);
      freeValue(targetVal); // valueToObject doesn't take ownership of targetVal!
      
      DISPATCH();
    }

    OP_INDEX_SET: {
      Value val = POP();
      Value idxVal = POP();
      Value targetVal = POP();

      if (!IS_INT(idxVal)) {
        VM_ERR(initTypeError, "Index must be an integer.");
        freeValue(val);
        freeValue(idxVal);
        freeValue(targetVal);
        HANDLE_ERROR(); 
      }

      int64_t i = AS_INT(idxVal);
      Object *target = valueToObject(targetVal);

      if (target->type == OBJ_LIST) {
        List *list = (List *)target;

        if (i >= 0 && (uint64_t)i < list->size) { 
          freeObject(list->objects[i]);
          list->objects[i] = valueToObject(val); 
        } else {
          VM_ERR(initIndexError, "Index out of range.");
          freeValue(val);
          freeValue(idxVal);
          freeValue(targetVal);
          HANDLE_ERROR(); 
        }

      } else if (target->type == OBJ_STRING) {
        String *str = (String *)target;
        Object *valObj = valueToObject(val);

        if (i >= 0 && (uint64_t)i < str->len && valObj->type == OBJ_STRING && ((String *)valObj)->len == 1) { 
          str->value[i] = ((String *)valObj)->value[0]; 
          freeObject(valObj);
        } else {
          VM_ERR(initIndexError, "Index out of range or invalid value.");
          freeObject(valObj);
          freeValue(idxVal);
          freeValue(targetVal);
          HANDLE_ERROR(); 
        }
      } else {
        VM_ERR(initTypeError, "Target is not indexable.");
        freeValue(val);
        freeValue(idxVal);
        freeValue(targetVal);
        HANDLE_ERROR(); 
      }

      PUSH(VAL_INT(1));
      freeValue(idxVal);
      freeValue(targetVal);
      
      DISPATCH();
    }

    OP_STORE_INDEX: {
      String *name = (String *)READ_CONST();
      Value val = POP();
      Value idxVal = POP();
      Object *target = getTable(vars, name->value);

      if (!target) {
        char buf[256]; snprintf(buf, sizeof(buf), "Undefined variable \"%s\".", name->value);
        VM_ERR(initNameError, buf);
        freeValue(val);
        freeValue(idxVal);
        HANDLE_ERROR();
      }

      if (!IS_INT(idxVal)) {
        VM_ERR(initTypeError, "Index must be an integer.");
        freeValue(val);
        freeValue(idxVal);
        HANDLE_ERROR();
      }

      int64_t i = AS_INT(idxVal);

      if (target->type == OBJ_LIST) {
        List *list = (List *)target;

        if (i >= 0 && (uint64_t)i < list->size) { 
          freeObject(list->objects[i]);
          list->objects[i] = valueToObject(copyValue(val));
        } else {
          VM_ERR(initIndexError, "Index out of range.");
          freeValue(val);
          freeValue(idxVal);
          HANDLE_ERROR();
        }

      } else if (target->type == OBJ_STRING) {
        String *str = (String *)target;
        Object *valObj = valueToObject(val);

        if (i >= 0 && (uint64_t)i < str->len && valObj->type == OBJ_STRING && ((String *)valObj)->len == 1) { 
          str->value[i] = ((String *)valObj)->value[0];
          freeObject(valObj);
        } else {
          VM_ERR(initIndexError, "Index out of range or invalid value.");
          freeObject(valObj);
          freeValue(idxVal);
          HANDLE_ERROR();
        }
      } else {
        VM_ERR(initTypeError, "Target is not indexable.");
        freeValue(val);
        freeValue(idxVal);
        HANDLE_ERROR();
      }

      PUSH(VAL_INT(1));
      freeValue(val);
      freeValue(idxVal);
      DISPATCH();
    }

    OP_CALL: {
      uint8_t argCount = READ_BYTE();
      Value args[256];

      for (int i = argCount - 1; i >= 0; i--) 
        args[i] = POP();

      Value calleeVal = POP();

      if (!IS_OBJ(calleeVal)) {
        VM_ERR(initTypeError, "Object is not callable.");
        HANDLE_ERROR();
      }

      Object *callee = valueToObject(calleeVal);

      if (callee->type == OBJ_FUNCTION) {
        Function *func = (Function *)callee;
          
        if (!func->chunk && func->body) {
          func->chunk = compileAST(func->body, vm->err, vm->filename, vm->sourcetext);
          if (func->chunk) func->maxLocals = func->chunk->maxLocals;
        }

        if (func->chunk) {
          if (vm->frameTop >= VM_CALL_STACK_MAX) {
            VM_ERR(initRuntimeError, "Call stack overflow.");
            freeValue(calleeVal); HANDLE_ERROR();
          }
          
          SAVE_STATE();
          
          CallFrame *newFrame = &vm->frames[vm->frameTop];
          
          newFrame->chunk = func->chunk;
          newFrame->ip = func->chunk->code;
          newFrame->variables = vars;
          newFrame->tryStackTop = vm->tryStackTop;
          newFrame->localsBase = vm->localsTop;
          newFrame->localCount = func->maxLocals;
          newFrame->currentInstr = 0;

          for (int i = 0; i < func->maxLocals; i++)
            if (i < (int)func->paramCount) {
              Value arg = (i < argCount) ? args[i] : VAL_INT(0);

              if (IS_OBJ(arg)) {
                Object* o = AS_OBJ(arg);

                if (o->type == OBJ_NUMBER_INT)
                  arg = VAL_INT(((Number*)o)->as.i);
                else if (o->type == OBJ_NUMBER_FLOAT)
                  arg = VAL_FLOAT(((Number*)o)->as.f);
              }

              vm->locals[newFrame->localsBase + i] = arg;
            } else 
              vm->locals[newFrame->localsBase + i] = VAL_UNDEF();
          
          for (int i = (int)func->paramCount; i < argCount; i++)
            freeValue(args[i]);

          vm->localsTop += func->maxLocals;
          vm->frameTop++;

          REFRESH_FRAME();
          freeValue(calleeVal);
          DISPATCH(); 
        }
      } else if (callee->type == OBJ_NATIVE_FUNCTION) {
        NativeFunction *nf = (NativeFunction *)callee;
        Object* objArgs[256];
        for (int i = 0; i < argCount; i++) objArgs[i] = valueToObject(args[i]);

        Object *res = nf->function(objArgs, argCount);

        for (int i = 0; i < argCount; i++) {
          if (!IS_OBJ(args[i])) freeObject(objArgs[i]);
          freeValue(args[i]);
        }
          
        if (res) {
          if (res->type == OBJ_ERROR) {
            VM_ERR(initRuntimeError, ((ProgramError*)res)->details);
            freeObject(res); 
            res = NULL;
          }

          if (res) 
            PUSH(objectToValue(res));
        }

        if (!res) { 
          freeValue(calleeVal);
          HANDLE_ERROR();
        }
      } else {
        freeValue(calleeVal);
        HANDLE_ERROR();
      }

      freeValue(calleeVal);
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
          PUSH(VAL_INT(1));
          found = true;
          break;
        }
      }

      if (found) DISPATCH();

      char *resolvedPath = resolveImportPath(vm->filename, name);
      char *fileContent = readFile(resolvedPath);

      if (!fileContent) {
        VM_ERR(initRuntimeError, "Failed to load imported file.");
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
        VM_ERR(initRuntimeError, "Call stack overflow.");
        HANDLE_ERROR();
      }

      SAVE_STATE();
      
      CallFrame* newFrame = &vm->frames[vm->frameTop];

      newFrame->chunk = chunk;
      newFrame->ip = chunk->code;
      newFrame->variables = vars;
      newFrame->tryStackTop = vm->tryStackTop;
      newFrame->localsBase = vm->localsTop;
      newFrame->localCount = chunk->maxLocals;
      newFrame->currentInstr = 0;

      for (int i = 0; i < chunk->maxLocals; i++)
        vm->locals[newFrame->localsBase + i] = VAL_UNDEF();

      vm->localsTop += chunk->maxLocals;
      vm->frameTop++;

      REFRESH_FRAME();

      DISPATCH();
    }

    OP_RETURN: {
      Value res = POP();

      if (vm->frameTop > 1) {
        CallFrame *leavingFrame = &vm->frames[--vm->frameTop];
        
        for (int i = 0; i < leavingFrame->localCount; i++)
          freeValue(vm->locals[leavingFrame->localsBase + i]);

        vm->localsTop = leavingFrame->localsBase;

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
      return valueToObject(res);
    }

    OP_BREAK: 
      return (Object *)initBreak();
    OP_CONTINUE: 
      return (Object *)initContinue();

    OP_HALT: {
      Value res = (sp > vm->stack) ? POP() : VAL_INT(0);

      if (vm->frameTop > 1) {
        CallFrame *leavingFrame = &vm->frames[--vm->frameTop];

        for (int i = 0; i < leavingFrame->localCount; i++)
          freeValue(vm->locals[leavingFrame->localsBase + i]);

        vm->localsTop = leavingFrame->localsBase;

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
      return valueToObject(res);
    }
  }
}

VM *initVM(Chunk *chunk, SymbolTable *variables, Error **err, char *filename, char *sourcetext) {
  VM *vm = arenaNew(objectArena, VM);
  if (!vm) return NULL;

  vm->frameTop = 0;

  vm->frames[vm->frameTop++] = (CallFrame){ 
    .chunk = chunk,
    .ip = chunk->code,
    .variables = variables,
    .tryStackTop = 0,
    .localsBase = 0,
    .localCount = chunk->maxLocals
  };

  vm->localsTop = chunk->maxLocals;

  vm->stackTop = 0;
  vm->tryStackTop = 0;
  vm->err = err;
  vm->filename = filename;
  vm->sourcetext = sourcetext;
  return vm;
}

void deinitVM(VM *vm) {
  if (!vm) return;

  for (int i = 0; i < vm->stackTop; i++) {
    freeValue(vm->stack[i]);
  }

  vm->stackTop = 0;

  while (vm->frameTop > 1) {
    CallFrame *frame = &vm->frames[--vm->frameTop];

    for (int i = 0; i < frame->localCount; i++) {
      freeValue(vm->locals[frame->localsBase + i]);
    }

    if (frame->variables != vm->frames[vm->frameTop - 1].variables) {
      freeTable(frame->variables);
    }
  }

  // Handle frame 0
  if (vm->frameTop == 1) {
    CallFrame *frame = &vm->frames[--vm->frameTop];
    for (int i = 0; i < frame->localCount; i++) {
      freeValue(vm->locals[frame->localsBase + i]);
    }
    // Note: frame 0's variables is global, freed in main.c
  }
}

