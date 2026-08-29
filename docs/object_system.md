# Object System - How Data Lives

Arc keeps two kinds of data. Small numbers live directly on the stack as a `Value`. Everything bigger lives on the heap as an `Object` and is pointed to by a `Value`. This keeps the hot path fast and keeps memory management simple.

## Value - The Stack Unit

Defined in `include/value.h`:

```c
typedef enum { VAL_UNDEF, VAL_NULL, VAL_INT, VAL_FLOAT, VAL_OBJ } ValueType;

typedef struct {
  ValueType type;
  union { int64_t i; double f; Object *obj; } as;
} Value;
```

* `VAL_INT` holds a full `int64_t` in `as.i`.
* `VAL_FLOAT` holds a `double` in `as.f`.
* `VAL_OBJ` holds a pointer to a heap `Object`.
* `VAL_NULL` and `VAL_UNDEF` are singletons. `VAL_UNDEF` means a local was created but not yet assigned and will raise `NameError: Variable used before assignment` if you read it.

Helpers: `VAL_INT(n)`, `VAL_FLOAT(n)`, `VAL_OBJ(o)`, `VAL_NULL()`, `VAL_UNDEF()`, plus `IS_INT(v)`, `IS_FLOAT(v)`, `IS_OBJ(v)`, `IS_NULL(v)`, `AS_INT(v)`, `AS_FLOAT(v)`, `AS_OBJ(v)`. `freeValue` and `copyValue` respect `isStatic` and `refCount`.

## Object - The Heap Header

Defined in `include/object.h`:

```c
typedef struct Object { ObjType type; int refCount; bool isStatic; } Object;
```

`ObjType` includes `OBJ_NUMBER_INT`, `OBJ_NUMBER_FLOAT`, `OBJ_STRING`, `OBJ_LIST`, `OBJ_FUNCTION`, `OBJ_NATIVE_FUNCTION`, `OBJ_CLASS`, `OBJ_INSTANCE`, `OBJ_FILE`, `OBJ_ERROR`, `OBJ_RETURN`, `OBJ_BREAK`, `OBJ_CONTINUE`, `OBJ_MODULE`, `OBJ_NULL`.

`isStatic` means the VM will not free it in `freeObject`. Constants in a `Chunk` are marked `isStatic = true`.

## What Each Object Looks Like

**Number** `src/objects/number.c` - `Number { Object base; union {i64,f} }`. Created by `initInt` and `initFloat` from `numberPool`. `copyNumber` dupes it.

**String** `src/objects/string.c` - `String { Object base; char *value; u64 len, capacity; long hash; bool isBuffer, ownsValue; }`. `initString` mallocs `len+1` and copies, `noCopyInitString` takes ownership, `initStringConst` points at an interned `internIdentifier` string with `ownsValue=false, isStatic=true`. `addString` grows `dest` if `capacity` allows else allocs a new `String` with `newCap = total*2`. `mulString` repeats with `SIZE_MAX/len` guard. Identifiers are interned in a global `IdentifierTable` with `IdentEntry {char *name; size_t len}` so `strlen` is not called on every probe.

**List** `src/objects/list.c` - `List { Object base; Object **objects; u64 size, capacity; }`. `initList` takes an `items` array, `copyList` bumps `refCount`. Lists are reference counted, not deep copied.

**Function** `src/objects/function.c` - `Function { Object base; char *name; char **params; size_t paramCount; ASTNode *body; Chunk *chunk; int maxLocals; }`. `initFunction` dups `name` and `params` with `stringDup` on `stringArena`. `copyFunction` shares `body` but not `chunk` to avoid double `freeChunk`.

**NativeFunction** - `NativeFunction { Object base; char *name; NativeFunc function; bool isVariadic; size_t requiredArgCount; }`. Created by `initNativeFunction` and registered in `registerBuiltins`.

**Class / Instance** `src/objects/class.c` - `Class { Object base; char *name; Chunk *chunk; int maxLocals; }`, `Instance { Object base; Class *klass; SymbolTable *fields; }`. `initClass` copies the class chunk, `initInstance` creates `fields` with `createTable`.

**File, Error, Return, Break, Continue, Module** - small wrappers with `initFile`, `initProgramError`, `initReturn`, `initBreak`, `initContinue`.

## Lifecycles

**Arenas** `src/memarena.c` - `parseArena`, `stringArena`, `objectArena`, `symbolPtrArena`, `poolArena`. `arenaAlloc` bumps `block->size` aligned to 8, `arenaRealloc` tries to grow in place at the tip else copies, `arenaReset` frees all blocks and clears `head` and `current`.

**Pools** `src/mempool.c` - `numberPool`, `stringPool`, `nativeFuncPool`, `functionPool`, `instancePool`, `symbolPool`, `symbolTablePool`. Each has a slab `POOL_SIZE 1024` plus `malloc` fallback, `poolAlloc` pops `slots[top-1]`, `poolFree` pushes or `free`s if not in slab. `initPool` checks `objSize > SIZE_MAX/POOL_SIZE` and `cap*2` overflow.

**Copy and free:** `copyObject` returns `isStatic` directly, otherwise `copyNumber`/`copyString` or `refCount++` for `LIST`/`INSTANCE`. `freeObject` switches on `type`: `NUMBER` and `STRING` go to pools (string checks `ownsValue`), `FUNCTION` frees its `Chunk`, `LIST` decrements `refCount` and frees elements that are not `isStatic` (strings check `ownsValue`), `INSTANCE` decrements and `freeTable`s `fields`. `forceFreeObject` is for shutdown and frees without the `isStatic` early exit.

**Chunks:** `Chunk` constants are `isStatic` while the chunk lives. `freeChunk` iterates `constants` and `poolFree`s strings or calls `freeObject` for others, then marks `freed`.

If you run with `-C`, `src/repl/main.c:61` `arcExit` will `freeTable(variables)`, `freeObject` the `argVect`, `freeMemPools`, and `freeArenas`.

