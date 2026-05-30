# Arc

Arc is a small programming language project written in C. It focuses on building a clean and reliable frontend pipeline, along with a simple interpreter for evaluating expressions and basic variable assignments.

## Documentation

For detailed information, please refer to the documentation in the `docs/` folder:

- [Getting Started](docs/getting-started.md)
- [Language Reference](docs/syntax.md)
- [Architecture](docs/architecture.md)

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

- [x] Lexer
- [x] Token system
- [x] REPL
- [x] Error reporting with position tracking
- [x] Parser (AST generation)
- [x] Expression evaluation
- [x] Variables and identifiers
- [x] String literals
- [x] Comparison and Logical operators
- [x] `IF`, `THEN`, `ELIF`, `ELSE` statements
- [x] `WHILE` loop
- [ ] Logical `NOT` operator
- [ ] Scoped environments
- [x] Functions
- [x] Return statement
- [x] Native Functions support
- [x] Basic runtime system
- [ ] Bytecode virtual machine (long-term goal)

## License

GPL-3.0
