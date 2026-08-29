# Arc Architecture - How the Pieces Fit

Arc is small on purpose. Source text goes through a short pipeline: lexing, parsing, compiling, and running. Each stage has a clear owner, clear memory, and clear errors. This doc walks that pipeline and the runtime that lives around it.

## The Pipeline at a Glance

1. **Lexing** `src/lexer.c` - text to `Token`s
2. **Parsing** `src/parser.c` - `Token`s to `ASTNode` tree
3. **Compiling** `src/compiler.c` - AST to `Chunk` bytecode
4. **Running** `src/vm.c` - `Chunk` on a stack VM

All four share `Position` (`line`, `column`, `index`) so errors and warnings can point at the exact spot.

## 1. Lexing - `include/lexer.h`, `src/lexer.c`

The `Lexer` holds `text`, `textLen`, `filename`, `pos`, and `currChar`. `initLexer` takes the filename and the file content (duplicated with `stringDup` on `stringArena`) and sets `pos` to `{-1,0,-1}`.

`lexNextToken` skips spaces, tabs, newlines, and comments that start with `#`, then dispatches:

* `"` → `makeStringLexer` - handles `\n`, `\t`, `\r`, `\0`, `\"`, `\\`, grows a small `stackBuf[256]` and falls back to `stringArena` if needed
* `'` → `makeCharLexer` - single ASCII char, `\'`, `\"`, `\\`, with proper errors for unknown escapes or empty char
* `!` → `makeNotEqualsToken` - expects `!=`
* `=` → `makeEqualsToken` - `=` or `==`
* `<` `>` → `makeLessThanToken` / `makeGreaterThanToken` - `<`, `<=`, `>`, `>=`
* `+ - * / ^ ( ) [ ] { } : , .` → single char tokens
* digit → `makeNumberTokenLexer` - scans `0-9` and at most one `.`, copies to a small buffer, then `strtoll` or `strtod` with `ERANGE` handling
* letter or `_` → `makeIdentifierLexer` - scans `alnum` and `_`, then `keywordType` does a case insensitive check. It uses a `CASE_MASK 0xDF...` and `loadKeyword64` to compare up to 8 chars without branching.

Keywords are case insensitive, identifiers are not. That check lives only here, so the parser and compiler never worry about case.

The lexer does not allocate a token array. It yields one `Token` at a time via `lexNextToken`, which keeps memory low.

## 2. Parsing - `include/parser.h`, `include/node.h`, `src/parser.c`

The parser is a recursive descent parser with Pratt style precedence for binary operators. It is built around:

* `Parser { Lexer *lexer; Token currentToken; Error **error; }`
* `ASTNode` with `getNodeStart` and `getNodeEnd` for every node type
* arena `parseArena` for all nodes and temporary arrays

**Precedence:** `getBinOpInfo` gives `OR/AND 1`, `== != < > <= >= 2`, `+ - 3`, `* / 4`, `^ 5` right associative. `parseExpr` and `continueExpr` use `minPrec` to build the tree correctly.

**Structure:** `parseExpr` → `parseExprPrimary` → `parseUnary` → `postfixParser` → `atomParser`. Then postfix handles calls, indexing, and property access in a loop:

* `(` → `initFunctionCallNode` with `args` grown from `4` to `N`
* `[` → `initIndexNode`
* `.` → `initPropertyAccessNode` (expects `IDENTIFIER` after `.`)

Other parsers: `parseIf` (handles `ELIF` and `ELSE`), `parseWhile`, `parseFor` (`FOR ident IN iterable THEN body END`), `parseFunction` (`FN name(params) THEN body END`), `parseClass` (`CLASS name body END`), `parseTryCatch` (`TRY body CATCH ident THEN handler END`), `parseReturn`, `parseImport` (`IMPORT "path"`), `parseVar` (`VAR` / `CONSTVAL` / `CONSTREF` `ident = expr`), and `parseIdentifier` for bare assignments.

`blockParser` and `parseProgram` collect statements until `EOF`, `ELIF`, `ELSE`, `END`, or `CATCH`. They start with small capacities (`parseProgram` uses `1024` to avoid realloc, `__blockParser` uses `64`, call args use `16`, lists use `64`) and grow with `arenaRealloc`. Depth is guarded by `MAX_DEPTH 8192` and a per parse `sDepth` counter that is reset per top level statement.

Errors go through `setError` which creates a `SyntaxError` with start, end, filename, and source text. The parser stops and returns `NULL` on first error.

## 3. Compiling - `include/compiler.h`, `src/compiler.c`

The compiler walks the AST and emits bytecode into a `Chunk`:

```c
struct Chunk {
  uint8_t *code; size_t count, capacity;
  Object **constants; size_t constCount, constCapacity;
  PosEntry *positions; size_t posCount, posCapacity;
  char *filename, *sourcetext;
  int maxLocals; bool freed;
}
```

**Positions:** `setPos` and `setPosFromNode` mark `posStart/posEnd/posDirty`. `emitByte` flushes a `PosEntry` when dirty, so `vmGetPos` can binary search `positions` by `currentInstr` to report errors.

**Locals:** `Compiler.locals[MAX_LOCALS 256]` tracks `name`, `len`, `slot`, `isMutable`, `isReference`. `resolveLocal` scans backwards with `strcmp`, `addLocal` appends. `c->isFunction` tells the compiler whether locals are allowed. At `FN` compile time a fresh `Compiler fc` is made, `maxLocals` is copied from `fc.maxLocalCount` to `func->chunk->maxLocals` and `func->maxLocals`.

**Constants:** `chunkAddConst` grows `constants` with `arenaRealloc` and marks `obj->isStatic = true`. Two dedup tables avoid bloat:

* `InternTable` for strings (`INTERN_TABLE_INIT_CAP 64`, `hashStr`, linear probe)
* `NumConstTable` for numbers (`NUM_CONST_TABLE_INIT_CAP 64`, `hashNumKey`)

`internString` interns the name, `addStringConst` does not, `addNumberConst` dedups floats and ints separately and handles `isFloat` vs `key.i/key.f`.

**Jumps:** `emitJump` writes `OP` and `0xFFFF`, `patchJump` fills the 16 bit offset later, `emitLoop` writes a backward `OP_JUMP`. This is used for `IF` (`JUMP_IF_FALSE` + `JUMP` chains), `WHILE` (`condition, JUMP_IF_FALSE exit, body, POP, JUMP loopStart`), `FOR` (`FOR_PREP, FOR_ITER, STORE_VAR, body, JUMP loopStart`), `TRY` (`TRY_PUSH catch, body, TRY_POP, JUMP end`), and `BREAK`/`CONTINUE` which are collected in `LoopInfo` and patched.

**Warnings:** `compileWarn` prints a yellow `Warning:` with file, line, column, the source line, and a `^` caret span. It is used for `BREAK`/`CONTINUE` outside a loop, `RETURN` outside a function, and unknown operators in `compileBinOp`/`compileUnaryOp`.

**Chunk lifecycle:** `initChunk` uses `CHUNK_INIT_CAP 256` and `CONST_INIT_CAP 64`, `chunkWrite` grows `code` by `*2`, `freeChunk` frees `String` values that `ownsValue` and `poolFree`s the rest. If `*err` is set, `compileAST` frees the chunk and returns `NULL`.

## 4. Running - `include/vm.h`, `src/vm.c`

The VM is a stack machine with computed goto:

```c
for(;;) { DISPATCH(); OP_ADD: { Value b=POP(); Value a=POP(); if(IS_INT(a)&&IS_INT(b)) {PUSH(VAL_INT(...)); DISPATCH();} ARITH_SLOW(...)} ... }
```

**State:** `LOAD_STATE` caches `frame`, `ip`, `vars`, `sp`, `constants` in registers. `SAVE_STATE` writes `sp` and `ip` back, `REFRESH_FRAME` reloads after a call that may have grown `frames`.

**Reading operands:** `_read_const_idx` loads the 24 bit constant index as `(p[0]<<16)|(p[1]<<8)|p[2]`, `_read_short` loads a 16 bit jump offset with `bswap16`. `DISPATCH` checks `op < dispatchSize` before the indirect jump.

**Stacks:** `VM.stack[VM_STACK_MAX 4096]`, `Value locals[VM_LOCALS_MAX 65536]`, `CallFrame frames[VM_CALL_STACK_MAX 8192]`, `TryFrame tryStack[VM_TRY_STACK_MAX 256]`. `PUSH` and `POP` are macros around `*sp++` and `*--sp`. `LOCAL(slot)` is `vm->locals[frame->localsBase+slot]`.

**Values:** `include/value.h` defines `Value { ValueType type; union {i64,f,ptr} }`. `VAL_INT` and `VAL_FLOAT` live directly on the stack with no allocation, so `int64_t` keeps all 64 bits. `isTruthy` checks `INT !=0`, `FLOAT !=0.0`, `NULL`/`UNDEF` false, empty string/list false.

**Arithmetic:** `OP_ADD` etc have a fast path for `IS_INT` on both sides, otherwise `doArith` handles mixed int/float, null, strings (`addString`, `mulString` with overflow guard `times>SIZE_MAX/len`), and `Number` objects. `doArith` is marked `cold` so the hot loop stays small.

**Locals vs globals:** `OP_LOAD_LOCAL`/`OP_STORE_LOCAL` use a slot, `OP_LOAD_VAR`/`OP_STORE_VAR`/`OP_DECLARE_VAR` use an interned `String*` name and `getTable/setTable/declareTable` on `SymbolTable`. `OP_STORE_VAR` respects `isMutable`, `OP_DECLARE_VAR` carries flags `0x1 mutable, 0x2 ref`.

**Control:** `OP_JUMP` adds `int16_t`, `OP_JUMP_IF_FALSE` pops and calls `isTruthy`, `OP_FOR_PREP` pushes `iterable,length,0`, `OP_FOR_ITER` checks `index<length`, pushes `item` (for strings via `initString` of one char) or jumps, `OP_BUILD_LIST` pops `N` values, `valueToObject`s them, and `initList`s, `OP_INDEX_GET/SET` work on strings and lists.

**Calls:** `OP_CALL` pops `callee` and `N` args. For `OBJ_FUNCTION` it checks `argCount==paramCount`, lazy compiles `func->body` if needed, checks `frameTop` and `localsTop`, saves state, builds a new `CallFrame` with `localsBase`, copies params, zeroes the rest to `VAL_UNDEF`, bumps `frameTop`/`localsTop`, and `DISPATCH`es. `OBJ_NATIVE_FUNCTION` boxes args to `Object**` (small buf `16` or `malloc`), calls `nf->function`, unboxes, handles `OBJ_ERROR`. `OBJ_CLASS` creates an `Instance` with `initInstance` and runs its chunk with `instance->fields` as `vars`.

**Errors:** `VM_ERR` and `VM_ERR_FRAME` set `*vm->err` with `vmGetPos` (binary search). `HANDLE_ERROR` unwinds `tryStack` and `frames`, frees locals, pushes the error string, and `continue`s to the catch `ip`. `OP_RETURN` and `OP_HALT` unwind one frame, restore `localsTop` and `tryStackTop`, and either `PUSH` the result and `DISPATCH` or `return valueToObject`.

**Memory:** `initVM` creates the first frame, `deinitVM` frees the stack and frames. With `-C` the VM frees `variables`, `argVect`, pools, and arenas on exit for leak checkers.

## Memory Management in Short

* **Arenas** `src/memarena.c` - `parseArena`, `stringArena`, `objectArena`, `symbolPtrArena`, `poolArena` - bump allocation, `arenaAlloc` aligns to 8, `arenaRealloc` tries in-place at tip else copies. `arenaReset` frees all blocks.
* **Pools** `src/mempool.c` - `numberPool`, `stringPool`, `nativeFuncPool`, `functionPool`, `instancePool`, `symbolPool`, `symbolTablePool` - slab of `POOL_SIZE 102 `4 plus `malloc` fallback, `poolAlloc`/`poolFree` with 64B align.
* **Objects** `src/object.c` - `isStatic` means never freed via `freeObject`, `copyObject` bumps `refCount` for `LIST`/`INSTANCE`, `freeObject` checks `ownsValue` for strings.

Together this keeps parsing and compiling cheap, and keeps hot runtime values on the stack without allocation.

