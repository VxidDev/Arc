# Architecture

Arc is intentionally designed to stay simple and easy to reason about. The main goals are:

* No external dependencies
* Clear separation of components:
  * Lexer
  * Tokens
  * Parser
  * AST nodes (including support for multiple statements)
  * Interpreter
  * Symbol table (for variables)
  * Error handling
  * Position tracking
  * REPL
* Predictable memory ownership
* Debuggability over cleverness

## Memory Safety

Arc is regularly tested with Valgrind to ensure:

* No memory leaks
* No invalid reads or writes
* Clear and consistent ownership rules

```bash
valgrind --leak-check=full --show-leak-kinds=all arc
```
