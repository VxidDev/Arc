# Arc

Arc is a small programming language project written in C. It currently focuses on building a clean and reliable frontend pipeline, including a lexer, REPL, and structured error reporting.

---

## Current Features

* Interactive REPL (`src/repl`)
* Lexer for arithmetic expressions
* Integer and floating-point number support
* Basic arithmetic operators:

  * `+`, `-`, `*`, `/`
  * parentheses `(` `)`
* Token system (`include/token.h`)
* Error handling with position tracking:

  * file name
  * line number
  * column number
* Focus on memory-safe design

---

## Example

```text
Arc > 123 + 45
[INT:123, PLUS, INT:45]

Arc > s
[ ARC - ERROR ] Illegal Character: 's'
File <stdin>, line 1, column 0
```

---

## Design Philosophy

Arc is intentionally designed to stay simple and easy to reason about. The main goals are:

* No external dependencies
* Clear separation of components:

  * lexer
  * tokens
  * errors
  * position tracking
  * REPL
* Predictable memory ownership

---

## Project Structure

```
include/
├── error.h
├── lexer.h
├── node.h
├── parser.h
├── position.h
├── repl/
│   └── input.h
├── token.h
└── utils.h

src/
├── error.c
├── lexer.c
├── node.c
├── parser.c
├── position.c
├── repl/
│   ├── input.c
│   └── main.c
├── token.c
└── utils.c
```

---

## Build

To build the project:

```bash
make
```

To run the REPL:

```bash
./arc
```

---

## Memory Safety

Arc is regularly tested with Valgrind to ensure:

* No memory leaks
* No invalid reads or writes
* Clear and consistent ownership rules

Run checks with:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./arc
```

---

## Roadmap

* Lexer (done)
* Token system (done)
* REPL (done)
* Error reporting with position tracking (done)
* Parser (AST generation)
* Expression evaluation
* Variables and identifiers
* Basic interpreter runtime
* Bytecode virtual machine (long-term goal)

---

## Notes

Arc is still early in development. The focus right now is correctness, memory safety, and building a solid foundation for parsing and evaluation before moving toward more advanced features.

---

## License

GPL-3.0
