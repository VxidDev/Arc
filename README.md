# Arc

Arc is a small programming language project written in C. It focuses on building a clean and reliable frontend pipeline, along with a simple interpreter for evaluating expressions and basic variable assignments.

---

## Current Features

* Interactive REPL (`src/repl`) with `clear` and `exit` commands
* Lexer for arithmetic expressions, identifiers, and **string literals**
* Parser with AST generation
* Expression evaluation (interpreter)
* Integer and floating-point number support
* String literal support
* Support for operations on strings:
  * `+`, `*`
* Variables and identifiers (using `VAR` keyword)
* Basic arithmetic operators:
  * `+`, `-`, `*`, `/`, `^`
  * parentheses `(` `)`
* Assignment operator `=`
* Token system (`include/token.h`)
* Structured error handling with position tracking:
  * file name
  * line number
  * column number
* Debug mode for inspecting tokens and AST
* Configurable floating-point precision
* Execute code from string via CLI
* Option to disable colored output
* Focus on memory-safe design

---

## Example

### Normal Mode

```text
Arc > VAR x = 5
5
Arc > x * 10
50
Arc > VAR name = "Arc Language"
Arc Language
Arc > VAR y = (x + 2) ^ 2
49.000000
Arc > y / 7
7.000000
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

Arc > VAR a = 10

Tokens: KEYWORD IDENTIFIER EQ INT 
AST tree: [a = 10]

10
```

---

### Error Handling

```text
Arc > s
Name Error: Undefined variable "s"
File <stdin>, line 1, column 1

s
^
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
  * symbol table (for variables)
  * error handling
  * position tracking
  * REPL
* Predictable memory ownership
* Debuggability over cleverness

---

## Project Structure

```
.
├── include
│   ├── ansi-colors.h
│   ├── error.h
│   ├── interpretator.h
│   ├── lexer.h
│   ├── node.h
│   ├── object.h
│   ├── parser.h
│   ├── position.h
│   ├── repl
│   │   ├── help.h
│   │   ├── input.h
│   │   ├── printast.h
│   │   └── repl.h
│   ├── symbol-table.h
│   ├── token.h
│   └── utils.h
├── LICENSE
├── makefile
├── README.md
└── src
    ├── error.c
    ├── interpretator.c
    ├── lexer.c
    ├── node.c
    ├── objects
    │   ├── number.c
    │   └── string.c
    ├── parser.c
    ├── position.c
    ├── repl
    │   ├── help.c
    │   ├── input.c
    │   ├── main.c
    │   └── printast.c
    ├── symbol-table.c
    ├── token.c
    └── utils.c
```

---

## Build

```bash
make
```

### Run

```bash
./arc
```

### Options

* **Debug mode**:
  ```bash
  ./arc -d
  # or
  ./arc --debug
  ```

* **Float precision**:
  ```bash
  ./arc -p 10
  # or
  ./arc --float-precision 10
  ```

* **Execute code**:
  ```bash
  ./arc -c "5 + 5"
  # or
  ./arc --code "VAR x = 10"
  ```

* **Disable colors**:
  ```bash
  ./arc -n
  # or
  ./arc --disable-colored-formatting
  ```

### Install

```bash
sudo make install
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

* [x] Lexer
* [x] Token system
* [x] REPL
* [x] Error reporting with position tracking
* [x] Parser (AST generation)
* [x] Expression evaluation
* [x] Variables and identifiers
* [x] String literals
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
