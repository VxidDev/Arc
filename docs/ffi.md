# Arc FFI and Dynamic Loading

The Arc Virtual Machine provides Foreign Function Interface (FFI) capabilities through dynamic library loading, allowing you to bridge Arc with external C libraries.

## High-Level Overview

Arc uses `dlopen`, `dlclose`, and `dlsym` internally to load shared objects (`.so` files) and lookup functions at runtime. This allows Arc programs to call into native C code dynamically.

## Integration Guide

To load and use an external C library in Arc:

1.  **Develop your C Library:** Ensure your functions follow the expected Arc native function signature:
    `Object* function_name(Object** args, size_t argCount)`
2.  **Compile as Shared Object:** Compile your C code as a position-independent shared object (`.so` file).
    `gcc -shared -fPIC -o mylib.so mylib.c`
3.  **Load in Arc:** Use the `dl_open` built-in function to open the `.so` file, and `dl_sym` to look up specific functions.

## API Reference (Built-ins)

These functions are available within Arc to interact with native libraries.

### `dl_open(string path, int mode)`
- **Description:** Loads a shared library.
- **Parameters:**
  - `path`: Path to the shared object file.
  - `mode`: Loading flags (e.g., `RTLD_LAZY`, `RTLD_NOW`).
- **Return Value:** Integer handle to the loaded library or `ProgramError` if loading fails.

### `dl_sym(int handle, string name, int paramCount, int isVariadic)`
- **Description:** Looks up a symbol in a loaded library and wraps it as a `NativeFunction` object.
- **Parameters:**
  - `handle`: Handle returned by `dl_open`.
  - `name`: Name of the symbol to look up.
  - `paramCount`: Number of arguments the function expects.
  - `isVariadic`: `1` if the function accepts a variable number of arguments, `0` otherwise.
- **Return Value:** A `NativeFunction` object or `ProgramError`.

### `dl_close(int handle)`
- **Description:** Closes a loaded library.
- **Parameters:**
  - `handle`: The handle of the library to close.
- **Return Value:** `1` on success, or `ProgramError`.

## Example

```c
// Arc code
let lib = dl_open("mylib.so", 1); // 1 = RTLD_LAZY
let myFunction = dl_sym(lib, "my_native_function", 2, 0);
myFunction(1, 2);
dl_close(lib);
```
