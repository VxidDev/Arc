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

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define LOAD_STATE() \
  CallFrame *frame = &vm->frames[vm->frameTop - 1]; \
  uint8_t *ip = frame->ip; \
  SymbolTable* vars = frame->variables; \
  register Value *sp = vm->stack + vm->stackTop; \
  Object** constants = frame->chunk->constants;

#define REFRESH_FRAME() do { \
  frame = &vm->frames[vm->frameTop - 1]; \
  ip = frame->ip; \
  vars = frame->variables; \
  sp = vm->stack + vm->stackTop; \
  constants = frame->chunk->constants; \
} while (0)

#define SAVE_STATE() do { \
  vm->stackTop = (int)(sp - vm->stack); \
  frame->ip = ip; \
} while (0)

#define PUSH(v) (*sp++ = (v))
#define POP() (*--sp)
#define TOP() (*(sp - 1))
#define PEEK(i) (*(sp - 1 - (i)))

#define READ_BYTE()  (*ip++)
#define READ_CONST() (constants[*ip++])


static inline int16_t _read_short(uint8_t **ipp) {
  uint16_t v;
  __builtin_memcpy(&v, *ipp, 2);
  *ipp += 2;
  return (int16_t)__builtin_bswap16(v);
}

static inline bool isTruthy(Value v) {
  if (IS_INT(v)) return AS_INT(v) != 0;
  if (IS_FLOAT(v)) return AS_FLOAT(v) != 0.0;

  if (IS_UNDEF(v) || IS_NULL(v)) return false;

  if (!IS_OBJ(v)) return true;

  Object *o = AS_OBJ(v);

  switch (o->type) {
    case OBJ_STRING:
      return ((String*)o)->len > 0;

    case OBJ_LIST:
      return ((List*)o)->size > 0;

    case OBJ_NUMBER_INT:
      return ((Number*)o)->as.i != 0;

    case OBJ_NUMBER_FLOAT:
      return ((Number*)o)->as.f != 0.0;

    default:
      return true;
  }
}

#define READ_SHORT() _read_short(&ip)

#define LOCAL(slot) (vm->locals[frame->localsBase + (slot)])

#define DISPATCH() do { \
  op = READ_BYTE(); \
  goto *dispatch[op]; \
} while(0)

#define VM_ERR(initFn, msg) do { \
  if (!*vm->err) { \
    frame->currentInstr = (uint32_t)(ip - frame->chunk->code); \
    Position _ps, _pe; \
    vmGetPos(frame, &_ps, &_pe); \
    *vm->err = initFn(_ps, _pe, frame->chunk->filename, msg, frame->chunk->sourcetext); \
  } \
} while(0)

#define VM_ERR_FRAME(vm, frame, initFn, msg) do { \
  if (!*(vm)->err) { \
    Position _ps, _pe; \
    vmGetPos(frame, &_ps, &_pe); \
    *(vm)->err = initFn(_ps, _pe, (frame)->chunk->filename, msg, (frame)->chunk->sourcetext); \
  } \
} while (0)

#define HANDLE_ERROR() \
  if (LIKELY(vm->tryStackTop > 0)) { \
    TryFrame tf = vm->tryStack[--vm->tryStackTop]; \
    while (vm->frameTop > tf.frameTop) { \
      CallFrame *leavingFrame = &vm->frames[--vm->frameTop]; \
      for (int i = 0; i < leavingFrame->localCount; i++) \
        freeValue(vm->locals[leavingFrame->localsBase + i]); \
      vm->localsTop = leavingFrame->localsBase; \
      if (!leavingFrame->instance && leavingFrame->variables != vm->frames[vm->frameTop - 1].variables) \
        freeTable(leavingFrame->variables); \
    } \
    REFRESH_FRAME(); \
    ip = tf.ip; \
    vm->stackTop = tf.stackTop; \
    sp = vm->stack + vm->stackTop; \
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

void freeValue(Value v) {
  if (IS_OBJ(v)) freeObject(AS_OBJ(v));
}

Value copyValue(Value v) {
  if (!IS_OBJ(v)) return v;

  Object* obj = AS_OBJ(v);
  if (obj->isStatic) return v;

  return VAL_OBJ(copyObject(obj));
}

static inline Value objectToValue(Object *o) {
  if (UNLIKELY(!o)) return VAL_INT(0);

  if (o->type == OBJ_NULL) return VAL_NULL();

  if (o->type == OBJ_NUMBER_INT) { 
    Value v = VAL_INT(((Number*)o)->as.i);
    freeObject(o);
    return v;
  }

  if (o->type == OBJ_NUMBER_FLOAT) { 
    Value v = VAL_FLOAT(((Number*)o)->as.f);
    freeObject(o);
    return v;
  }

  return VAL_OBJ(o);
}

static inline Object *valueToObject(Value v) {
  if (IS_INT(v)) return (Object*)initInt(AS_INT(v));
  if (IS_FLOAT(v)) return (Object*)initFloat(AS_FLOAT(v));
  if (IS_NULL(v)) return (Object*)initNull();
  return AS_OBJ(v);
}

static void vmGetPos(CallFrame *frame, Position *start, Position *end) {
  Chunk *chunk = frame->chunk;
  PosEntry *entries = chunk->positions;
  size_t count = chunk->posCount;

  if (UNLIKELY(count == 0)) {
    *start = *end = (Position){0, 0, 0};
    return;
  }

  size_t lo = 0, hi = count;

  while (lo < hi) {
    size_t mid = (lo + hi) >> 1;
    if (entries[mid].offset <= frame->currentInstr) lo = mid + 1;
    else hi = mid;
  }

  size_t idx = lo ? lo - 1 : 0;
  *start = entries[idx].start;
  *end = entries[idx].end;
}

static Value doArith(VM *vm, CallFrame* frame, OpCode op, Value a, Value b) {
  if ((IS_INT(a) || IS_FLOAT(a)) && (IS_INT(b) || IS_FLOAT(b))) {
    double na = IS_INT(a) ? (double)AS_INT(a) : AS_FLOAT(a);
    double nb = IS_INT(b) ? (double)AS_INT(b) : AS_FLOAT(b);

    switch (op) {
      case OP_ADD: return VAL_FLOAT(na + nb);
      case OP_SUB: return VAL_FLOAT(na - nb);
      case OP_MUL: return VAL_FLOAT(na * nb);
      case OP_DIV: 
        if (UNLIKELY(nb == 0.0)) {
          VM_ERR_FRAME(vm, frame, initValueError, "Division by zero.");
          return VAL_UNDEF();
        }

        return VAL_FLOAT(na / nb);
      case OP_POW: return VAL_FLOAT(pow(na, nb));
      case OP_EQ: return VAL_INT(na == nb); 
      case OP_NE: return VAL_INT(na != nb); 
      case OP_LT: return VAL_INT(na < nb); 
      case OP_GT: return VAL_INT(na > nb); 
      case OP_LTE: return VAL_INT(na <= nb); 
      case OP_GTE: return VAL_INT(na >= nb); 
      case OP_AND: return VAL_INT((int)(na) && (int)(nb)); 
      case OP_OR: return VAL_INT((int)(na) || (int)(nb)); 
      default: break;
    }
  }

  if (IS_NULL(a) || IS_NULL(b)) {
    if (op == OP_EQ) return VAL_INT(IS_NULL(a) && IS_NULL(b));
    if (op == OP_NE) return VAL_INT(!(IS_NULL(a) && IS_NULL(b)));

    VM_ERR_FRAME(vm, frame, initTypeError, "Incompatible types for operation.");
    freeValue(a);
    freeValue(b);
    return VAL_UNDEF();
  }

  // Handle strings
  if (!IS_OBJ(a) && !IS_OBJ(b)) {
    VM_ERR_FRAME(vm, frame, initTypeError, "Incompatible types for operation.");
    return VAL_UNDEF();
  }

  Object* aObj = IS_OBJ(a) ? AS_OBJ(a) : NULL;
  Object* bObj = IS_OBJ(b) ? AS_OBJ(b) : NULL;

  bool aStr = aObj && aObj->type == OBJ_STRING;
  bool bStr = bObj && bObj->type == OBJ_STRING; 

  if (aStr || bStr) {
    Value res = VAL_UNDEF();

    if (op == OP_ADD && aStr && bStr) 
      res = VAL_OBJ((Object *)addString((String *)aObj, (String *)bObj));
    else if (op == OP_MUL && aStr && (IS_INT(b) || IS_FLOAT(b))) {
      Number tmp;
      tmp.as.i = IS_INT(b) ? AS_INT(b) : (int64_t)AS_FLOAT(b);
      tmp.base.type = OBJ_NUMBER_INT;
      res = VAL_OBJ((Object *)mulString((String *)aObj, &tmp));
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

    if (UNLIKELY(result != ERR_NONE)) {
      if (!*vm->err) {
        if (result == ERR_DIV_BY_ZERO) VM_ERR_FRAME(vm, frame, initValueError, "Division by zero.");
        else VM_ERR_FRAME(vm, frame, initTypeError, "Incompatible types for operation.");
      }

      freeValue(b);
      return VAL_INT(0);
    }

    return objectToValue((Object*)nb);
  }

  VM_ERR_FRAME(vm, frame, initTypeError, "Incompatible types for operation.");
  freeValue(a);
  freeValue(b);
  return VAL_UNDEF();
}

Object *vmRun(VM *vm) {
  OpCode op;
  LOAD_STATE();

  static const void *dispatch[] = {
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

    [OP_CALL] = &&OP_CALL,

    [OP_TRY_PUSH] = &&OP_TRY_PUSH,
    [OP_TRY_POP] = &&OP_TRY_POP,

    [OP_IMPORT] = &&OP_IMPORT,

    [OP_RETURN] = &&OP_RETURN,
    [OP_BREAK] = &&OP_BREAK,
    [OP_CONTINUE] = &&OP_CONTINUE,
    [OP_HALT] = &&OP_HALT,

    [OP_LOAD_LOCAL] = &&OP_LOAD_LOCAL,
    [OP_STORE_LOCAL] = &&OP_STORE_LOCAL,

    [OP_PROPERTY_ACCESS] = &&OP_PROPERTY_ACCESS,
    [OP_PROPERTY_SET] = &&OP_PROPERTY_SET
  };

  for (;;) {
    DISPATCH();

    OP_LOAD_CONST: {
      Object *c = READ_CONST();
      
      if (c->type == OBJ_NULL) {
        PUSH(VAL_NULL());
      } else if (c->type == OBJ_NUMBER_INT)
        PUSH(VAL_INT(((Number*)c)->as.i));
      else if (c->type == OBJ_NUMBER_FLOAT)
        PUSH(VAL_FLOAT(((Number*)c)->as.f));
      else if (c->isStatic || c->type == OBJ_FUNCTION || c->type == OBJ_STRING)
        PUSH(VAL_OBJ(c)); // constants are owned by chunk
      else
        PUSH(copyValue(objectToValue(c)));

      DISPATCH();
    } 

    OP_LOAD_VAR: {
      String *name = (String *)READ_CONST();
      Value val = getTable(vars, name->value);

      if (UNLIKELY(IS_UNDEF(val))) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Undefined variable \"%s\".", name->value);
        VM_ERR(initNameError, buf); 
        HANDLE_ERROR();
      }
 
      PUSH(IS_OBJ(val) && !AS_OBJ(val)->isStatic ? copyValue(val) : val);
      DISPATCH();
    }

    OP_PROPERTY_ACCESS: {
      String* name = (String*)READ_CONST();
      Value targetVal = POP();

      if (UNLIKELY(!IS_OBJ(targetVal) || AS_OBJ(targetVal)->type != OBJ_INSTANCE)) {
        VM_ERR(initTypeError, "Only instances have properties.");
        freeValue(targetVal);
        HANDLE_ERROR();
      }

      Instance* target = (Instance*)AS_OBJ(targetVal);
      Value val = getTable(target->fields, name->value);

      if (UNLIKELY(IS_UNDEF(val))) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Instance has no property \"%s\".", name->value);
        VM_ERR(initNameError, buf);
        freeValue(targetVal);
        HANDLE_ERROR();
      }

      PUSH(IS_OBJ(val) && !AS_OBJ(val)->isStatic ? copyValue(val) : val);
      freeValue(targetVal);
      DISPATCH();
    }

    OP_PROPERTY_SET: {
      String* name = (String*)READ_CONST();
      
      Value val = POP();
      Value targetVal = POP();

      if (UNLIKELY(!IS_OBJ(targetVal) || AS_OBJ(targetVal)->type != OBJ_INSTANCE)) {
        VM_ERR(initTypeError, "Only instances have properties.");
        freeValue(targetVal);
        freeValue(val);
        HANDLE_ERROR();
      }

      Instance* target = (Instance*)AS_OBJ(targetVal);
      setTable(target->fields, name->value, val);

      PUSH(val);
      freeValue(targetVal);

      DISPATCH();
    }
      
    OP_STORE_VAR: {
      String *name = (String *)READ_CONST();
  
      if (UNLIKELY(_DEBUG)) {
        printf("[debug] OP_STORE_VAR: name = %p | name->value = %p\n", name, name ? name->value : (void*)0);
      }

      setTable(vars, name->value, PEEK(0));
      DISPATCH();
    }

    OP_LOAD_LOCAL: {
      uint8_t slot = READ_BYTE();
      Value val = LOCAL(slot);

      if (UNLIKELY(IS_UNDEF(val))) {
        VM_ERR(initNameError, "Variable used before assignment.");
        HANDLE_ERROR();
      }

      PUSH((IS_OBJ(val) && AS_OBJ(val)->isStatic) ? val : IS_OBJ(val) ? copyValue(val) : val); 
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
      freeValue(POP());
      DISPATCH();
    }

    
  #define ARITH_SLOW(OP_ENUM) \
    { Value res = doArith(vm, frame, OP_ENUM, a, b); \
      if (UNLIKELY(*vm->err)) { freeValue(res); HANDLE_ERROR(); } \
      PUSH(res); DISPATCH(); }
 
    OP_ADD: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) + AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_ADD);
    }
    OP_SUB: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) - AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_SUB);
    }
    OP_MUL: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) * AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_MUL);
    }
    OP_DIV: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) {
        int64_t nb = AS_INT(b);
        if (UNLIKELY(nb == 0)) { VM_ERR(initValueError, "Division by zero."); HANDLE_ERROR(); }
        PUSH(VAL_FLOAT((double)AS_INT(a) / nb)); DISPATCH();
      }
      ARITH_SLOW(OP_DIV);
    }
    OP_POW: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) {
        PUSH(VAL_FLOAT(pow((double)AS_INT(a), (double)AS_INT(b)))); DISPATCH();
      }
      ARITH_SLOW(OP_POW);
    }
    OP_EQ: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) == AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_EQ);
    }
    OP_NE: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) != AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_NE);
    }
    OP_LT: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) < AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_LT);
    }
    OP_GT: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) > AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_GT);
    }
    OP_LTE: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) <= AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_LTE);
    }
    OP_GTE: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) >= AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_GTE);
    }
    OP_AND: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) && AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_AND);
    }
    OP_OR: {
      Value b = POP(); Value a = POP();
      if (LIKELY(IS_INT(a) && IS_INT(b))) { PUSH(VAL_INT(AS_INT(a) || AS_INT(b))); DISPATCH(); }
      ARITH_SLOW(OP_OR);
    }
  #undef ARITH_SLOW

    OP_NEG: {
      Value a = POP();
      
      if (LIKELY(a.type == VAL_INT)) 
        PUSH(VAL_INT(-AS_INT(a)));
      else if (a.type == VAL_FLOAT) 
        PUSH(VAL_FLOAT(-AS_FLOAT(a)));
      else { 
        VM_ERR(initTypeError, "Operand must be a number.");
        freeValue(a); 
        HANDLE_ERROR();
      }

      DISPATCH();
    }

    OP_NOT: {
      Value a = POP();

      if (LIKELY(a.type == VAL_INT)) 
        PUSH(VAL_INT(!AS_INT(a)));
      else if (a.type == VAL_FLOAT) 
        PUSH(VAL_INT(!AS_FLOAT(a)));
      else { 
        VM_ERR(initTypeError, "Operand must be a number.");
        freeValue(a);
        HANDLE_ERROR();
      }

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

      if (!isTruthy(cond))
        ip += offset;

      DISPATCH();
    }

    OP_FOR_PREP: {
      Value iterVal = POP();

      if (UNLIKELY(!IS_OBJ(iterVal))) {
        VM_ERR(initTypeError, "Object is not iterable");
        freeValue(iterVal);
        HANDLE_ERROR();
      }

      Object* iterable = AS_OBJ(iterVal);

      if (UNLIKELY(iterable->type != OBJ_LIST && iterable->type != OBJ_STRING)) {
        VM_ERR(initTypeError, "Object is not iterable");
        freeValue(iterVal); 
        HANDLE_ERROR(); 
      }
      
      int64_t len = (iterable->type == OBJ_LIST) ? (int64_t)((List*)iterable)->size : (int64_t)((String*)iterable)->len;

      PUSH(iterVal); // iterable (sp - 3)
      PUSH(VAL_INT(len)); // length (sp - 2)
      PUSH(VAL_INT(0)); // index (sp - 1)
      
      DISPATCH();
    }

    OP_FOR_ITER: {
      int16_t offset = READ_SHORT();

      int64_t index = AS_INT(PEEK(0));
      int64_t length = AS_INT(PEEK(1));
      
      if (LIKELY(index < length)) {
        Object *iterable = AS_OBJ(PEEK(2));
        Value item;

        if (iterable->type == OBJ_LIST) {
          Object *elem = ((List *)iterable)->objects[index];

          if (elem->type == OBJ_NUMBER_INT)
            item = VAL_INT(((Number*)elem)->as.i);
          else if (elem->type == OBJ_NUMBER_FLOAT)
            item = VAL_FLOAT(((Number*)elem)->as.f);
          else
            item = copyValue(objectToValue(elem));
        } else { 
          char buf[2] = { ((String *)iterable)->value[index], '\0' };
          item = VAL_OBJ((Object *)initString(buf, 1));
        }

        PUSH(item);
        (sp - 2)->as.i = index + 1; 
      } else {
        (void)POP(); // index, primitive 
        (void)POP(); // length, primitive 
        freeValue(POP()); // iterVal, owns list/string copy 
        ip += offset;
      }
      
      DISPATCH();
    }

    OP_BUILD_LIST: {
      uint8_t count = READ_BYTE();

      Object* smallBuf[64];
      Object** items = (count <= 64) ? smallBuf : (Object**)malloc(sizeof(Object*) * count);

      for (int i = 0; i < count; i++) 
        items[count - 1 - i] = valueToObject(PEEK(i));
      
      sp -= count;

      PUSH(VAL_OBJ((Object*)initList(items, count, count)));
      if (items != smallBuf) free(items);

      DISPATCH();
    }

    OP_INDEX_GET: {
      Value idxVal = POP();
      Value targetVal = POP();

      if (UNLIKELY(!IS_INT(idxVal))) { 
        VM_ERR(initIndexError, "Index must be an integer."); 
        freeValue(idxVal);
        freeValue(targetVal);
        HANDLE_ERROR(); 
      }

      int64_t i = AS_INT(idxVal);
      Object *target = AS_OBJ(targetVal); 
      
      if (target->type == OBJ_STRING) {
        String *str = (String *)target;

        if (UNLIKELY(i < 0 || (uint64_t)i >= str->len)) { 
          VM_ERR(initIndexError, "Index out of range."); 
          freeValue(idxVal);
          freeValue(targetVal);
          HANDLE_ERROR(); 
        }

        char buf[2] = { str->value[i], '\0' };
        PUSH(VAL_OBJ((Object *)initString(buf, 1)));

      } else if (LIKELY(target->type == OBJ_LIST)) {
        List *list = (List *)target;

        if (UNLIKELY(i < 0 || (uint64_t)i >= list->size)) { 
          VM_ERR(initIndexError, "Index out of range.");
          freeValue(idxVal);
          freeValue(targetVal);
          HANDLE_ERROR(); 
        }

        PUSH(copyValue(objectToValue(list->objects[i])));
      } else { 
        VM_ERR(initTypeError, "Target is not indexable.");
        freeValue(idxVal);
        freeValue(targetVal);
        HANDLE_ERROR(); 
      }
      
      freeValue(targetVal); 
      DISPATCH();
    }

    OP_INDEX_SET: {
      Value val = POP();
      Value idxVal = POP();
      Value targetVal = POP();

      if (UNLIKELY(!IS_INT(idxVal))) {
        VM_ERR(initTypeError, "Index must be an integer.");
        freeValue(val);
        freeValue(idxVal);
        freeValue(targetVal);
        HANDLE_ERROR(); 
      }

      int64_t i = AS_INT(idxVal);
      Object *target = AS_OBJ(targetVal);

      if (LIKELY(target->type == OBJ_LIST)) {
        List *list = (List *)target;

        if (UNLIKELY(i < 0 && (uint64_t)i >= list->size)) { 
          VM_ERR(initIndexError, "Index out of range.");
          freeValue(val);
          freeValue(idxVal);
          freeValue(targetVal);
          HANDLE_ERROR();  
        }

        freeObject(list->objects[i]);
        list->objects[i] = valueToObject(val); 
      } else if (target->type == OBJ_STRING) {
        String *str = (String *)target;
        Object *valObj = valueToObject(val);

        if (UNLIKELY(i < 0 || (uint64_t)i >= str->len || valObj->type != OBJ_STRING || ((String *)valObj)->len != 1)) {
          VM_ERR(initIndexError, "Index out of range or invalid value.");
          freeObject(valObj);
          freeValue(idxVal);
          freeValue(targetVal);
          HANDLE_ERROR(); 
        }

        str->value[i] = ((String *)valObj)->value[0]; 
        freeObject(valObj);
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

    OP_CALL: {
      uint8_t argCount = READ_BYTE();

      frame->currentInstr = (uint32_t)(ip - frame->chunk->code - 2);
      Value *args = sp - argCount;
      sp -= argCount;

      Value calleeVal = POP();

      if (UNLIKELY(!IS_OBJ(calleeVal))) {
        VM_ERR(initTypeError, "Object is not callable.");
        HANDLE_ERROR();
      }

      Object *callee = AS_OBJ(calleeVal);

      if (LIKELY(callee->type == OBJ_FUNCTION)) {
        Function *func = (Function *)callee;

        if (UNLIKELY(argCount != func->paramCount)) {
          char buf[256];
          snprintf(buf, sizeof(buf), "Function \"%s\" expects %zu arguments, got %d.", func->name, func->paramCount, argCount);
          VM_ERR(initRuntimeError, buf);
          freeValue(calleeVal);
          HANDLE_ERROR();
        }
          
        if (UNLIKELY(!func->chunk && func->body)) {
          func->chunk = compileAST(func->body, vm->err, vm->filename, vm->sourcetext);
          if (func->chunk) func->maxLocals = func->chunk->maxLocals;
          func->body = NULL;
        }

        if (UNLIKELY(!func->chunk)) {
          freeValue(calleeVal);
          VM_ERR(initRuntimeError, "Bytecode chunk is null. (internal error)");
          HANDLE_ERROR();
        }

        if (vm->frameTop >= VM_CALL_STACK_MAX) {
          VM_ERR(initRuntimeError, "Call stack overflow.");
          freeValue(calleeVal);
          HANDLE_ERROR();
        }

        if (UNLIKELY(vm->localsTop + (int)func->maxLocals > VM_LOCALS_MAX)) {
          VM_ERR(initRuntimeError, "Locals stack overflow.");
          freeValue(calleeVal);
          HANDLE_ERROR();
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
        newFrame->instance = NULL;
        newFrame->filename = frame->filename;
        
        int paramCount = (int)func->paramCount;
        int base = newFrame->localsBase;
        int i = 0;

        for (; i < paramCount && i < argCount; i++) {
          vm->locals[base + i] = args[i];
        } 

        for (; i < func->maxLocals; i++)
          vm->locals[newFrame->localsBase + i] = VAL_UNDEF();

        for (int i = (int)func->paramCount; i < argCount; i++)
          freeValue(args[i]);

        vm->localsTop += func->maxLocals;
        vm->frameTop++;

        if (!callee->isStatic) freeValue(calleeVal);

        REFRESH_FRAME();
        DISPATCH(); 
      }

      if (callee->type == OBJ_NATIVE_FUNCTION) {
        NativeFunction *nf = (NativeFunction *)callee;
        
        if (argCount < nf->requiredArgCount) {
          char buf[256];
          snprintf(buf, sizeof(buf), "Function \"%s\" expects atleast %zu arguments, got %d.", nf->name, nf->requiredArgCount, argCount);
          VM_ERR(initRuntimeError, buf);
          freeValue(calleeVal);
          HANDLE_ERROR();
        }

        Object* objArgsBuf[16];
        Object** objArgs = (argCount <= 16) ? objArgsBuf : (Object**)malloc(sizeof(Object*) * argCount);

        for (int i = 0; i < argCount; i++)
          objArgs[i] = valueToObject(args[i]);

        Object *res = nf->function(objArgs, argCount);

        for (int i = 0; i < argCount; i++) {
          if (!IS_OBJ(args[i])) 
            freeObject(objArgs[i]);
          else if (objArgs[i] != res) 
            freeValue(args[i]);
        }

        if (objArgs != objArgsBuf) free(objArgs);
          
        if (UNLIKELY(res && res->type == OBJ_ERROR)) {
          VM_ERR(initRuntimeError, ((ProgramError*)res)->details);
          freeObject(res); 
          res = NULL;
        }

        freeValue(calleeVal);

        if (UNLIKELY(!res)) {
          VM_ERR(initRuntimeError, "Native function returned null.");
          HANDLE_ERROR()
        }

        PUSH(objectToValue(res));
        DISPATCH();
      }

      if (callee->type == OBJ_CLASS) {
        Class* class = (Class*)callee;

        for (int i = 0; i < argCount; i++) {
          freeValue(args[i]);
        }

        if (UNLIKELY(!class->chunk)) {
          VM_ERR(initRuntimeError, "Class has no body.");
          freeValue(calleeVal);
          HANDLE_ERROR();
        }
        
        if (UNLIKELY(vm->frameTop >= VM_CALL_STACK_MAX)) {
          VM_ERR(initRuntimeError, "Call stack overflow.");
          freeValue(calleeVal);
          HANDLE_ERROR();
        }

        if (UNLIKELY(vm->localsTop + (int)class->maxLocals > VM_LOCALS_MAX)) {
          VM_ERR(initRuntimeError, "Locals stack overflow.");
          freeValue(calleeVal);
          HANDLE_ERROR();
        }

        Instance* instance = initInstance(class, vars);

        if (UNLIKELY(!instance)) {
          VM_ERR(initRuntimeError, "Out of memory.");
          freeValue(calleeVal);
          HANDLE_ERROR();
        }
        
        CallFrame* newFrame = &vm->frames[vm->frameTop];
        
        SAVE_STATE();

        newFrame->chunk = class->chunk;
        newFrame->ip = class->chunk->code;
        newFrame->variables = instance->fields;
        newFrame->tryStackTop = vm->tryStackTop;
        newFrame->localsBase = vm->localsTop;
        newFrame->localCount = 0;
        newFrame->currentInstr = 0;
        newFrame->instance = (Object*)instance;
        newFrame->filename = frame->filename;

        for (int i = 0; i < class->maxLocals; i++)
          vm->locals[newFrame->localsBase + i] = VAL_UNDEF();

        vm->localsTop += class->maxLocals;
        vm->frameTop++;

        if (!callee->isStatic) freeValue(calleeVal);

        REFRESH_FRAME();
        DISPATCH();
      }

      VM_ERR(initTypeError, "Object is not callable.");
      freeValue(calleeVal);
      HANDLE_ERROR();
    }

    OP_TRY_PUSH: {
      int16_t offset = READ_SHORT();

      if (LIKELY(vm->tryStackTop < VM_TRY_STACK_MAX)) {
        vm->tryStack[vm->tryStackTop++] = (TryFrame){
          .ip = ip + offset,
          .frameTop = vm->frameTop,
          .stackTop = (int)(sp - vm->stack)
        };
      }

      DISPATCH();
    }

    OP_TRY_POP:
      if (vm->tryStackTop > 0) vm->tryStackTop--;
      DISPATCH();

    OP_IMPORT: {
      String *pathObj = (String *)READ_CONST();
      char *name = pathObj->value;

      for (size_t i = 0; stdlibModules[i]; i++) {
        if (strcmp(stdlibModules[i]->name, name) == 0) {
          stdlibModules[i]->init(vars);
          PUSH(VAL_INT(1));
          DISPATCH();
        }
      }

      char *resolvedPath = resolveImportPath(frame->filename, name);
      
      if (_DEBUG) { 
        printf("[debug] vm->filename: %s\n", vm->filename);
        printf("[debug] frame->filename: %s\n", frame->filename);
        printf("[debug] name: %s\n", name);
        printf("[debug] Resolved import path: %s\n", resolvedPath);
      }

      char *fileContent = readFile(resolvedPath);

      if (UNLIKELY(!fileContent)) {
        VM_ERR(initRuntimeError, "Failed to load imported file.");
        HANDLE_ERROR(); 
      }

      Lexer *lexer = initLexer(stringDup(name), fileContent);
      size_t tokenAmount = 0;
      Token *tokens = makeTokensLexer(lexer, vm->err, &tokenAmount);

      if (UNLIKELY(!tokens)) {
        HANDLE_ERROR();
      }

      Parser *parser = initParser(tokens, tokenAmount, vm->err, fileContent, name);
      ASTNode *ast = parseProgram(parser);

      if (UNLIKELY(!ast)) { 
        HANDLE_ERROR();
      }

      Chunk *chunk = compileAST(ast, vm->err, name, fileContent);

      if (UNLIKELY(!chunk)) { 
        HANDLE_ERROR();
      }

      if (UNLIKELY(vm->frameTop >= VM_CALL_STACK_MAX)) {
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
      newFrame->instance = NULL;
      newFrame->filename = resolvedPath;

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
        
        if (!leavingFrame->instance && !leavingFrame->instance && leavingFrame->variables != vm->frames[vm->frameTop - 1].variables)
          freeTable(leavingFrame->variables);

        vm->tryStackTop = leavingFrame->tryStackTop;
        
        SAVE_STATE();
        REFRESH_FRAME();

        vm->filename = frame->filename;

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
        
        if (leavingFrame->instance) {
          freeValue(res);
          res = VAL_OBJ(leavingFrame->instance);
        } else if (leavingFrame->variables != vm->frames[vm->frameTop - 1].variables) {
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
    .localCount = chunk->maxLocals,
    .instance = NULL,
    .filename = filename
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

    for (int i = 0; i < frame->localCount; i++)
      freeValue(vm->locals[frame->localsBase + i]);

    if (frame->variables != vm->frames[vm->frameTop - 1].variables)
      freeTable(frame->variables);
  }

  if (vm->frameTop == 1) {
    CallFrame *frame = &vm->frames[--vm->frameTop];

    for (int i = 0; i < frame->localCount; i++)
      freeValue(vm->locals[frame->localsBase + i]);
  }
}

