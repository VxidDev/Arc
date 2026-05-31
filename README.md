# Arc

Arc is a small programming language project written in C. It focuses on building a clean and reliable frontend pipeline, along with a simple interpreter for evaluating expressions and basic variable assignments.

## Documentation

For detailed information, please refer to the documentation in the `docs/` folder:

- [Getting Started](docs/getting-started.md)
- [Language Reference](docs/syntax.md)
- [Architecture](docs/architecture.md)

## Features

Arc is a C-based programming language with a focus on a clean frontend pipeline and a simple interpreter. Key implemented features include:

*   **Variables and Assignment:** Declare and update variables using the `VAR` keyword.
*   **Functions:** Define custom functions with parameters and return values using the `FN` and `RETURN` keywords.
*   **Native Functions:** Support for built-in functions implemented in C for core functionalities like I/O, type checking, and error handling.
*   **Control Flow:**
    *   **Conditional Statements:** `IF`, `THEN`, `ELIF`, `ELSE` for branching logic.
    *   **Loops:** `WHILE` loops for repetitive execution.
           * **Control Flow Statements**: `BREAK` and `CONTINUE` for loop control.
*   **Operators:** Full support for arithmetic (`+`, `-`, `*`, `/`, `^`), comparison (`==`, `!=`, `<`, `>`, `<=`, `>=`), and logical (`AND`, `OR`) operations.
*   **Data Types:** Supports numbers, strings, and booleans.
*   **Error Handling:** Robust error reporting with position tracking.
*   **Read-Eval-Print Loop (REPL):** An interactive environment for executing Arc code directly.

## Example

### File Execution

Create a file named `script.arc`:

```arc
VAR x = 0

WHILE x < 5 THEN
    VAR x = x + 1
END

IF x == 5 THEN
    print("Five")
ELSE
    print("Not Five")
END
```

Running this file (`./arc script.arc`) will output:

```text
Five
```

## Roadmap

*   Logical `NOT` operator
*   Scoped environments
*   Bytecode virtual machine (long-term goal)

## License

GPL-3.0
