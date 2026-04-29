# Arc

Arc is a small programming language project written in C. It focuses on building a clean and reliable frontend pipeline, along with a simple interpreter for evaluating expressions.

---

## Current Features

* Interactive REPL (`src/repl`)
* Lexer for arithmetic expressions
* Parser with AST generation
* Expression evaluation (interpreter)
* Integer and floating-point number support
* Basic arithmetic operators:

  * `+`, `-`, `*`, `/`
  * parentheses `(` `)`
* Token system (`include/token.h`)
* Structured error handling with position tracking:

  * file name
  * line number
  * column number
* Debug mode for inspecting tokens and AST
* Focus on memory-safe design

---

## Example

### Normal Mode

```text
Arc > 3-3
0
Arc > 3*3*3
27
```

---

### Debug Mode

Run with:

```bash
./arc -d
# or
./arc --debug
```

Example session:

```text
Arc > 1-1

Tokens: INT MINUS INT 
AST tree: (1 MINUS 1)

0

Arc > 3*3*3

Tokens: INT MUL INT MUL INT 
AST tree: ((3 MUL 3) MUL 3)

27
```

---

### Error Handling

```text
Arc > s
Illegal Character: 's'
File <stdin>, line 1, column 0
```

---

## Design Philosophy

Arc is intentionally designed to stay simple and easy to reason about. The main goals are:

* No external dependencies
* Clear separation of components:

  * lexer
  * tokens
  * parser
  * AST nodes
  * interpreter
  * error handling
  * position tracking
  * REPL
* Predictable memory ownership
* Debuggability over cleverness

---

## Project Structure

```
.
├── arc
├── include
│   ├── ansi-colors.h
│   ├── error.h
│   ├── interpretator.h
│   ├── lexer.h
│   ├── node.h
│   ├── object.h
│   ├── parser.h
│   ├── position.h
│   ├── repl/
│   │   └── input.h
│   ├── token.h
│   └── utils.h
├── src
│   ├── error.c
│   ├── interpretator.c
│   ├── lexer.c
│   ├── node.c
│   ├── objects/
│   │   └── number.c
│   ├── parser.c
│   ├── position.c
│   ├── repl/
│   │   ├── input.c
│   │   └── main.c
│   ├── token.c
│   └── utils.c
├── makefile
├── README.md
└── LICENSE
```

---

## Build

```bash
make
```

Run:

```bash
./arc
```

Debug mode:

```bash
./arc -d
# or
./arc --debug
```

To install:

```bash
sudo make install
```

Run:
```
arc 
# or 
arc -d 
# or 
arc --debug 
```
```
```
```
```

---

## Memory Safety

Arc is regularly tested with Valgrind to ensure:

* No memory leaks
* No invalid reads or writes
* Clear and consistent ownership rules

```bash
valgrind --leak-check=full --show-leak-kinds=all ./arc
```

---

## Roadmap

* Lexer (done)
* Token system (done)
* REPL (done)
* Error reporting with position tracking (done)
* Parser (AST generation) (done)
* Expression evaluation (done)
* Variables and identifiers
* Scoped environments
* Functions
* Basic runtime system
* Bytecode virtual machine (long-term goal)

---

## Notes

Arc is still early in development. The current focus is correctness, memory safety, and building a solid foundation for parsing and evaluation before moving toward more advanced language features.

---

## License

GPL-3.0
