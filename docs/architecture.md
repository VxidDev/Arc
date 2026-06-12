# Arc Architecture

Arc is a stack-based virtual machine and compiler designed for simplicity and performance. It follows a classic pipeline: source code is tokenized, parsed into an Abstract Syntax Tree (AST), compiled into custom bytecode, and finally executed on a VM.

## Execution Pipeline

1.  **Lexical Analysis (`src/lexer.c`)**: Converts the raw source text into a stream of `Token` structures. Keywords are case-insensitive (e.g., `VAR` and `var` are identical).
2.  **Syntax Analysis (`src/parser.c`)**: A recursive descent parser that builds an **Abstract Syntax Tree (AST)** composed of `ASTNode` structures.
3.  **Compilation (`src/compiler.c`)**: Traverses the AST and emits custom **Bytecode** into a `Chunk`. This phase performs local variable resolution, constant interning, and jump patching for control flow.
4.  **Virtual Machine (`src/vm.c`)**: A stack-based executor that runs the bytecode. It uses a high-performance "computed goto" dispatch loop.

## Virtual Machine Design

- **Type**: Stack-based. Operands are pushed onto and popped from a central data stack.
- **Dispatch**: Uses a labels-as-values jump table for opcode dispatch, significantly faster than a traditional `switch` statement in C.
- **Call Stack**: Managed by `CallFrame` structures. Each function call or module import creates a new frame with its own Instruction Pointer (`ip`) and local variable array.
- **Symbol Tables**: Global variables and module-level symbols are stored in hash tables (`SymbolTable`).
- **Data Stack**: A fixed-size array of `Object*` pointers (default limit is 4096).

## Memory Management

Arc employs a hybrid memory management strategy focused on performance and low fragmentation:

- **Arena Allocation (`src/memarena.c`)**: Used for persistent data structures that live for the duration of a phase (like the AST during parsing).
- **Memory Pools (`src/mempool.c`)**: Used for frequently allocated runtime objects like `Number` and `String`. This reduces the overhead of `malloc` and `free`.
- **Manual Reference Management**: The VM handles object lifecycles using `copyObject` and `freeObject`. There is currently no automated generational garbage collector; instead, the stack and symbol tables own references to objects.

## Object System

Every value in Arc is an `Object` with a specific `ObjType`:
- **Numbers**: Supports 64-bit integers and double-precision floats. Small integers (-128 to 127) are cached and statically allocated.
- **Strings**: Stored in a dedicated string arena. Identifiers (variable names) are interned to allow for fast pointer-comparison lookups.
- **Lists**: Dynamically sized arrays of `Object` pointers.
- **Functions**: First-class objects containing a reference to their compiled bytecode `Chunk`.
- **Native Functions**: C function pointers wrapped as Arc objects for standard library integration.
