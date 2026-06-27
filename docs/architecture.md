# Arc Architecture

Arc is a stack-based virtual machine and compiler designed for simplicity and performance. It follows a classic pipeline: source code is tokenized, parsed into an Abstract Syntax Tree (AST), compiled into custom bytecode, and finally executed on a VM.

## Execution Pipeline

1.  **Lexical Analysis (`src/lexer.c`)**: Converts the raw source text into a stream of `Token` structures. Keywords are case-insensitive.
2.  **Syntax Analysis (`src/parser.c`)**: A recursive descent parser that builds an **Abstract Syntax Tree (AST)** composed of `ASTNode` structures.
3.  **Compilation (`src/compiler.c`)**: Traverses the AST and emits custom **Bytecode** into a `Chunk`. This phase performs local variable resolution, constant interning, and jump patching for control flow.
4.  **Virtual Machine (`src/vm.c`)**: A stack-based executor that runs the bytecode. It uses a high-performance "computed goto" dispatch loop.

## Virtual Machine Design

- **Type**: Stack-based. Operands are pushed onto and popped from a central data stack.
- **Dispatch**: Uses a labels-as-values jump table for opcode dispatch.
- **Call Stack**: Managed by `CallFrame` structures. Each function call or module import creates a new frame with its own Instruction Pointer (`ip`) and local variable array.
- **Symbol Tables**: Global variables and module-level symbols are stored in hash tables (`SymbolTable`).
- **Data Stack**: A fixed-size array of `Value` structures.

## Memory Management

Arc employs a hybrid memory management strategy focused on performance and low fragmentation:

- **Arena Allocation (`src/memarena.c`)**: Used for persistent data structures that live for the duration of a phase (like the AST during parsing).
- **Memory Pools (`src/mempool.c`)**: Used for frequently allocated runtime objects like `Number` and `String`. 
- **Manual Reference Management**: The VM handles object lifecycles using `freeObject` and `forceFreeObject`.

## Value System

The Arc VM uses a `Value` structure as the primary unit of data for the stack, locals, and runtime operations. It is designed as a tagged union to efficiently represent different types within the same memory footprint.

```c
typedef enum {
  VAL_UNDEF, // Undefined value
  VAL_NULL,
  VAL_INT,
  VAL_FLOAT,
  VAL_OBJ,   // Pointer to heap-allocated object
} ValueType;

typedef struct {
  ValueType type;

  union {
    int64_t i;
    double f;
    Object *obj;
  } as;
} Value;
```

- `VAL_INT` and `VAL_FLOAT` represent numeric types stored directly in the structure.
- `VAL_OBJ` acts as a container for all heap-allocated data (Strings, Lists, Classes, etc.), handled by the Arc Object System.
- `VAL_UNDEF` and `VAL_NULL` represent special states.

