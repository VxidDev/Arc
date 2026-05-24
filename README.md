# Arc

Arc is a small programming language project written in C. It focuses on building a clean and reliable frontend pipeline, along with a simple interpreter for evaluating expressions and basic variable assignments.

---

## Current Features

* Interactive REPL (`src/repl`) with `clear` and `exit` commands
* File execution (supports `.arc` files)
* Lexer for arithmetic expressions, identifiers, and **string literals**
* Parser with AST generation, supporting multiple statements
* Expression evaluation (interpreter)
* Integer and floating-point number support
* String literal support
* List literal support
* Support for operations on strings:
  * `+` (concatenation), `*` (repetition)
* Support for operations on lists:
  * Indexing
  * Slicing (TODO)
* Functions:
  * Define functions using the `FN` keyword.
  * Supports parameters.
  * Call functions using `function_name(arg1, arg2)`.

* Variables and identifiers (using `VAR` keyword)
* **Conditional statements**: `IF`, `THEN`, `ELIF`, `ELSE`, `END`
* **Looping**: `WHILE`, `THEN`, `END`
* **Multi-line support**: Bodies of `IF`, `ELIF`, `ELSE`, and `WHILE` can contain multiple statements.
* Basic arithmetic operators:
  * `+`, `-`, `*`, `/`, `^`
  * Parentheses `(` `)`
* Comparison operators:
  * `==`, `!=`, `<`, `>`, `<=`, `>=`
* Logical operators:
  * `AND`, `OR`
* Assignment operator `=`
* Token system (`include/token.h`)
* Structured error handling with position tracking:
  * File name
  * Line number
  * Column number
* Debug mode for inspecting tokens and AST
* Configurable floating-point precision
* Execute code from string via CLI
* Option to disable colored output
* Focus on memory-safe design

---

## Example

### Normal Mode

```text
### File Execution Example

Create a file named `script.arc`:

```text
VAR x = 0

WHILE x < 5 THEN
    x = x + 1
END

IF x == 5 THEN
    "Five"
ELSE
    "Not Five"
END
```

Running this file (`./arc script.arc`) will output:

```text
Five
```

Arc Arc Arc 
Arc > "Hello " + "World"
Hello World
Arc > VAR y = (x + 2) ^ 2
49.000000
Arc > y / 7
7.000000
Arc > x == 5
1
Arc > x > 10 OR x == 5 
1
```

---

## Command-line Arguments

Arc provides access to command-line arguments through built-in variables:

* `argc`: The number of positional arguments (including the script file).
* `argv`: List of positional arguments. (including the script file).

Example usage in a script:
```text
VAR i = 1

IF argc > 1 THEN
    argv[0]
ELSE
    "No arguments provided"
END
```

If you run `arc script.arc hello world`:
* `argc` will be `3`
* `argv[0]` will be `"script.arc"` (or the path to the executable)
* `argv[1]` will be `"hello"`
* `argv[2]` will be `"world"`

---

### Functions

```text
FN add(a, b) THEN
    a + b
END

VAR result = add(5, 3)
result
```

Running this code will output:

```text
8
```

---

## Debug Mode

Run with:

```bash
arc -d
# or
arc --debug
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
    ├── object.c
    ├── objects
    │   ├── list.c
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

Arc uses a `makefile` with several build targets:

* **dev** (default): Builds with optimizations disabled and debug symbols enabled.
  ```bash
  make dev
  # or simply
  make
  ```

* **debug**: Builds with AddressSanitizer and UndefinedBehaviorSanitizer for deep debugging.
  ```bash
  make debug
  ```

* **release**: Builds with full optimizations (`-O3`, `-flto`, `-march=native`) for maximum performance.
  ```bash
  make release
  ```

### Run

Arc can be run using the `arc` command if installed, or via the local executable.

```bash
arc
# or
./arc
```

### Options

* **Debug mode**:
  ```bash
  arc -d
  # or
  arc --debug
  ```

* **Float precision**:
  ```bash
  arc -p 10
  # or
  arc --float-precision 10
  ```

* **Execute code**:
  ```bash
  arc -c "5 + 5"
  # or
  arc --code "VAR x = 10"
  ```

* **Disable colors**:
  ```bash
  arc -n
  # or
  arc --disable-colored-formatting
  ```

* **Run a file**:
  ```bash
  arc script.arc
  ```

### Install

The `install` target builds the **release** version and installs it to `/usr/bin` (by default).

```bash
sudo make install
```

You can also install specific versions:
```bash
sudo make dev-install
sudo make debug-install
sudo make release-install
```

### Uninstall

```bash
sudo make uninstall
```

---

## Memory Safety

Arc is regularly tested with Valgrind to ensure:

* No memory leaks
* No invalid reads or writes
* Clear and consistent ownership rules

```bash
valgrind --leak-check=full --show-leak-kinds=all arc
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
* [x] Comparison and Logical operators
* [x] `IF`, `THEN`, `ELIF`, `ELSE` statements
* [x] `WHILE` loop
* [ ] Logical `NOT` operator
* [ ] Scoped environments
* [x] Functions
* [ ] Basic runtime system
* [ ] Bytecode virtual machine (long-term goal)

---

## Notes

Arc is still early in development. The current focus is correctness, memory safety, and building a solid foundation for parsing and evaluation before moving toward more advanced language features.

---

## License

GPL-3.0
