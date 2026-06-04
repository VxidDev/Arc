# Architecture

Arc is intentionally designed to stay simple and easy to reason about. The main goals are:

## Components

Arc is intentionally designed to stay simple and easy to reason about. The main components include:

*   **Lexer**: Tokenizes the input source code.
*   **Parser**: Performs recursive descent parsing to construct an Abstract Syntax Tree (AST).
*   **AST Nodes**: Represents the structure of the program, supporting multiple statements, expressions, and control flow.
*   **Interpreter**: Visits the AST nodes to execute the program logic.
*   **Symbol Table**: Manages variable scopes and function definitions.
*   **Built-in Functions**: Core functions implemented in C for performance and system access.
*   **Memory Management**: Custom memory pooling for efficient object allocation and reuse.
*   **Error Handling**: Detailed error reporting with position tracking (file, line, column).
*   **REPL**: An interactive environment for rapid development and testing.

## Memory Management

Arc uses a custom memory pooling system to manage the allocation of frequently used objects (like numbers and strings).

*   **Object Pooling**: Instead of frequent calls to `malloc` and `free`, Arc maintains pools of pre-allocated memory slots. When an object is "freed", it is returned to the pool for later reuse.
*   **Efficiency**: This reduces fragmentation and improves performance for programs that create and destroy many small objects.
*   **Configurable**: The memory pool size can be adjusted via command-line arguments (`-m` or `--mempool-size`).

## Memory Safety

Arc is regularly tested with Valgrind to ensure:

* No memory leaks
* No invalid reads or writes
* Clear and consistent ownership rules

```bash
valgrind --leak-check=full --show-leak-kinds=all ./arc script.arc
```
