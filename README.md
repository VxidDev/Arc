# Arc

Arc is a modern, stack-based bytecode virtual machine and programming language designed for simplicity and performance.

## Documentation

For detailed information, please refer to the documentation in the `docs/` folder, starting with [docs/index.md](docs/index.md).

## Features

*   **Modern VM Architecture:** Compiled to custom bytecode and executed on a high-performance stack-based virtual machine.
*   **Clean Syntax:** Intuitive, case-insensitive keywords for control flow.
*   **Object-Oriented:** Supports classes with dynamic field access and modularity.
*   **Memory Management:** Hybrid strategy using arenas, pools, and manual reference management.
*   **FFI:** Built-in capability to load and interact with external native C libraries (`.so`).
*   **Standard Library:** Rich standard library covering I/O, math, JSON, and filesystem utilities.
*   **Robust Error Handling:** Position-aware error reporting with `TRY...CATCH` blocks.

## Build and Install

```bash
# Build (default dev)
make

# Build release (optimized)
make release

# Install to /usr/bin
sudo make install
```

## Example

```arc
VAR list = [1, 2, 3, 4, 5]

FOR item IN list THEN
    IF item == 3 THEN
        CONTINUE
    END
    print("Item: ", item)
END
```

## License

GPL-3.0
