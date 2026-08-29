# Bytecode Reference - What the VM Runs

Bytecode is defined in `include/compiler.h` as `OpCode` and implemented in `src/vm.c` as a computed goto table. A `Chunk` holds `code` bytes, a `constants` pool, and a `positions` map for errors. Operands that refer to constants use a 24 bit index `(b1<<16)|(b2<<8)|b3`, jumps use a 16 bit signed offset.

## How to Read This

* **Stack:** what the opcode pops and pushes. `...` is the rest of the stack.
* **Operands:** bytes that follow the opcode.
* **Notes:** what it does and what errors it can raise.

## Data Movement

**`OP_LOAD_CONST`**
* Stack: `... -> ..., value`
* Operands: 24b const index
* Does: `PUSH(constants[idx])`. If the constant is `NULL` it pushes `VAL_NULL`, if it is an `int` or `float` it pushes `VAL_INT`/`VAL_FLOAT` directly, otherwise it pushes the object. Constants are `isStatic`.

**`OP_LOAD_VAR`**
* Stack: `... -> ..., value`
* Operands: 24b interned name index
* Does: `getTable(vars, name->value)` and pushes a copy. If `IS_UNDEF` it raises `NameError: Undefined variable "x"`.

**`OP_STORE_VAR`**
* Stack: `..., value -> ..., value` (peeks, does not pop)
* Operands: 24b name index
* Does: `peek = PEEK(0); setTable(vars,name,peek)` or `setTableLocal` when inside a class instance. If `setTable` returns false (constant) it raises `NameError: Cannot assign to constant`.

**`OP_DECLARE_VAR`**
* Stack: `..., value -> ..., value` (peeks)
* Operands: 24b name index + 1B flags (`0x1 mutable`, `0x2 ref`)
* Does: `declareTable(vars,name,peek,isMutable,isReference)` or `setTableLocal` for instances.

**`OP_LOAD_LOCAL`**
* Stack: `... -> ..., value`
* Operands: 8b slot
* Does: `val = locals[base+slot]; if IS_UNDEF raise NameError: Variable used before assignment; PUSH(copyValue(val))` unless `isStatic`.

**`OP_STORE_LOCAL`**
* Stack: `..., value -> ..., value` (peeks)
* Operands: 8b slot
* Does: unboxes `int`/`float` from `Object*` if needed, `freeValue(old)`, `locals[base+slot]=copyValue(peek)`.

**`OP_POP`**
* Stack: `..., value -> ...`
* Does: `freeValue(POP())`.

## Arithmetic and Logic

All of these pop `a` and `b` (or `a` for unary) and push the result. The fast path checks `IS_INT` on both sides first, otherwise `doArith` handles mixed types.

**`OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_POW`**
* Stack: `..., a, b -> ..., result`
* Does: `a + b` etc. `OP_DIV` checks `b==0` and raises `ValueError: Division by zero`. `OP_POW` uses `pow(double,double)`. String `+` concatenates via `addString`, string `* int` repeats via `mulString` with `SIZE_MAX/len` guard.

**`OP_EQ, OP_NE, OP_LT, OP_GT, OP_LTE, OP_GTE`**
* Stack: `..., a, b -> ..., 0 or 1`
* Does: numeric compare, null compare, or string compare with `len` check then `memcmp`. `EQ`/`NE` on non-string, non-number returns `false`/`true` without error.

**`OP_AND, OP_OR`**
* Stack: `..., a, b -> ..., 0 or 1`
* Does: `int(a) && int(b)` etc.

**`OP_NEG`**
* Stack: `..., a -> ..., -a`
* Does: if `VAL_INT` then `-i`, if `VAL_FLOAT` then `-f`, else `TypeError: Operand must be a number`.

**`OP_NOT`**
* Stack: `..., a -> ..., !a`
* Does: logical not for `VAL_INT`/`VAL_FLOAT`, else `TypeError`.

## Control Flow

**`OP_JUMP`**
* Operands: 16b signed offset
* Does: `ip += offset`.

**`OP_JUMP_IF_FALSE`**
* Stack: `..., cond -> ...` (pops)
* Operands: 16b offset
* Does: `if (!isTruthy(POP())) ip += offset; freeValue(cond)`.

**`OP_FOR_PREP`**
* Stack: `..., iterable -> ..., iterable, length, index`
* Does: expects `iterable` is `OBJ_LIST` or `OBJ_STRING`, pushes the object back, then `VAL_INT(len)` and `VAL_INT(0)`.

**`OP_FOR_ITER`**
* Stack: `..., iterable, length, index -> ..., iterable, length, index, item` or `...,  (cleaned)`
* Operands: 16b exit offset
* Does: if `index < length` pushes `item` (`list->objects[index]` or `initString` of one char) and bumps `index`, else pops `index,length,iterable` (freeing iterable) and jumps.

**`OP_TRY_PUSH`**
* Operands: 16b offset to catch
* Does: pushes `TryFrame{ip+offset, frameTop, stackTop}` onto `tryStack` if not full.

**`OP_TRY_POP`**
* Does: pops `tryStack` if any.

**`OP_BREAK` / `OP_CONTINUE`**
* Returned by `vmRun` as `BREAK`/`CONTINUE` objects and handled at compile time as jumps. At runtime they just return.

## Functions, Classes, and Properties

**`OP_CALL`**
* Stack: `..., callee, arg1..argN -> ..., result`
* Operands: 8b `N`
* Does: pops `callee`. If `OBJ_FUNCTION` checks `N==paramCount`, lazy compiles `func->body` if `chunk==NULL`, checks `frameTop` and `localsTop`, saves state, builds a new `CallFrame` with `variables` or `instance->fields`, copies args, zeroes remaining locals to `VAL_UNDEF`, bumps `frameTop`/`localsTop`, and dispatches. If `OBJ_NATIVE_FUNCTION` checks `isVariadic` vs `requiredArgCount`, boxes `Value` args to `Object**`, calls `nf->function`, unboxes, handles `OBJ_ERROR`. If `OBJ_CLASS` creates an `Instance` via `initInstance` and runs its chunk.

**`OP_RETURN`**
* Stack: `..., result -> ...` (to caller)
* Does: pops `result`, unwinds one `CallFrame`, frees its locals, restores `localsTop` and `tryStackTop`, handles `instance` vs `variables`, frees `ownsChunk`, pushes `result` to caller or returns `valueToObject(result)`.

**`OP_PROPERTY_ACCESS`**
* Stack: `..., instance -> ..., value`
* Operands: 24b name index
* Does: expects `OBJ_INSTANCE`, `getTableLocal(fields,name)`, pushes copy, frees instance.

**`OP_PROPERTY_SET`**
* Stack: `..., instance, value -> ..., value`
* Operands: 24b name index
* Does: expects `OBJ_INSTANCE`, `setTableLocal(fields,name,value)`, pushes `value`, frees instance.

## Collections

**`OP_BUILD_LIST`**
* Stack: `..., v1..vN -> ..., list`
* Operands: 24b `N`
* Does: checks `N <= VM_STACK_MAX` and `sp-N >= stack`, allocates `items` (`smallBuf[64]` or `malloc`), `valueToObject`s each `PEEK`, pops `N`, `initList`s, pushes `list`.

**`OP_INDEX_GET`**
* Stack: `..., target, index -> ..., value`
* Does: expects `index` is `VAL_INT`, target is `String` or `List`. For string pushes `initString` of one char, for list pushes `copyValue` of `objects[i]`. Out of range raises `IndexError`.

**`OP_INDEX_SET`**
* Stack: `..., target, index, value -> ..., 1`
* Does: for `List` it `freeObject`s old `objects[i]` and stores `valueToObject(value)`. For `String` it expects `value` is a one-char string and `i` in range, then `str->value[i]=char`. Pushes `1`.

## Imports and Halt

**`OP_IMPORT`**
* Operands: 24b path index
* Does: first scans `stdlibModules` for a native name and `init`s it. Otherwise `resolveImportPath` from `frame->filename`, `readFile`, `initLexer`, `parseProgram`, `compileAST`, checks `frameTop`, builds a new `CallFrame` with `ownsChunk=true` and the same `vars`, and dispatches.

**`OP_DECLARE_VAR` vs `OP_STORE_VAR`:** `DECLARE` is the first `VAR` or `CONSTVAL`/`CONSTREF` for that name, `STORE` is a later plain assignment.

**`OP_HALT`**
* Does: pops result if any else `0`, unwinds one frame if `frameTop > exitFrameTop` (handling instance vs variables and `ownsChunk`), pushes result and dispatches, otherwise `return valueToObject(result)` and exits `vmRun`.

All jumps are patched by `emitJump`/`patchJump`/`emitLoop` at compile time, so offsets are already correct when the VM sees them.

