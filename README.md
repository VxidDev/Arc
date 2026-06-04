# Arc

Arc is a small programming language project written in C. It focuses on building a clean and reliable frontend pipeline, along with a simple interpreter for evaluating expressions and basic variable assignments.

## Documentation

For detailed information, please refer to the documentation in the `docs/` folder:

- [Getting Started](docs/getting-started.md)
- [Language Reference](docs/syntax.md)
- [Architecture](docs/architecture.md)

## Project Structure

```text
.
├── docs
│   ├── architecture.md
│   ├── getting-started.md
│   ├── index.md
│   └── syntax.md
├── include
│   ├── ansi-colors.h
│   ├── builtIns
│   │   ├── errors.h
│   │   ├── io.h
│   │   ├── lists.h
│   │   ├── math.h
│   │   ├── properties.h
│   │   ├── string.h
│   │   └── typing.h
│   ├── builtIns.h
│   ├── error.h
│   ├── interpretator.h
│   ├── lexer.h
│   ├── mempool.h
│   ├── node.h
│   ├── object.h
│   ├── parser.h
│   ├── position.h
│   ├── repl
│   │   ├── help.h
│   │   ├── input.h
│   │   ├── printast.h
│   │   ├── readfile.h
│   │   └── repl.h
│   ├── symbol-table.h
│   ├── token.h
│   └── utils.h
├── LICENSE
├── makefile
├── math.arc
├── README.md
└── src
    ├── builtIns
    │   ├── errors.c
    │   ├── io.c
    │   ├── lists.c
    │   ├── math.c
    │   ├── properties.c
    │   ├── string.c
    │   └── typing.c
    ├── builtIns.c
    ├── error.c
    ├── interpretator.c
    ├── lexer.c
    ├── mempool.c
    ├── node.c
    ├── object.c
    ├── objects
    │   ├── break.c
    │   ├── continue.c
    │   ├── error.c
    │   ├── file.c
    │   ├── function.c
    │   ├── list.c
    │   ├── module.c
    │   ├── number.c
    │   ├── return.c
    │   └── string.c
    ├── parser.c
    ├── position.c
    ├── repl
    │   ├── help.c
    │   ├── input.c
    │   ├── main.c
    │   ├── printast.c
    │   ├── readfile.c
    │   └── repl.c
    ├── symbol-table.c
    ├── token.c
    └── utils.c
```

## Features

Arc is a C-based programming language with a focus on a clean frontend pipeline and a simple interpreter. Key implemented features include:

*   **Variables and Assignment:** Declare and update variables using the `VAR` keyword.
*   **Functions:** Define custom functions with parameters and return values using the `FN` and `RETURN` keywords.
*   **Built-in Functions:** Support for built-in functions implemented in C for core functionalities like I/O, type checking, list manipulation, and error handling.
*   **Control Flow:**
    *   **Conditional Statements:** `IF`, `THEN`, `ELIF`, `ELSE` for branching logic.
    *   **Loops:** `WHILE` and `FOR` loops for repetitive execution.
    *   **Loop Control:** `BREAK` and `CONTINUE` statements.
*   **Exception Handling:** `TRY...CATCH` blocks for handling runtime errors.
*   **Data Types:** Supports numbers (integers and floats), strings, booleans, and lists.
*   **Import System:** Modularize code using the `IMPORT` keyword.
*   **Memory Management:** Uses custom memory pools for efficient object allocation.
*   **Error Handling:** Robust error reporting with position tracking (file, line, column).
*   **REPL:** Interactive environment with syntax highlighting (via ANSI colors) and command-line options.

## Example

### File Execution

Create a file named `script.arc`:

```arc
VAR list = [1, 2, 3, 4, 5]

FOR item IN list THEN
    IF item == 3 THEN
        CONTINUE
    END
    print("Item: " + item)
END
```

Running this file (`./arc script.arc`) will output:

```text
Item: 1
Item: 2
Item: 4
Item: 5
```


## Roadmap

*   Proper re-declaring and type-check
*   Better standard library
*   Scoped environments (currently variables are re-declared in same scope)
*   Bytecode virtual machine (long-term goal)

## License

GPL-3.0
