# Arc - v0.5.0-beta

Arc is a small, stack based programming language that compiles to its own bytecode and runs on a fast virtual machine. It is easy to read, easy to hack, and built to be practical. You write simple code, it gets compiled to compact bytecode, and the VM runs it with a direct dispatch loop, pooled objects, and arena memory.

## Start Here

If you are new, read the docs in this order:

1. [Documentation Index](docs/index.md)
2. [Getting Started](docs/getting-started.md) - how to build, run, and install
3. [Language Reference](docs/syntax.md) - what the language looks like
4. [Architecture](docs/architecture.md) - how the compiler and VM fit together
5. [Bytecode Reference](docs/bytecode.md) - what the VM actually executes
6. [Object System](docs/object_system.md) - how values live in memory
7. [FFI](docs/ffi.md) - how to call C libraries

Standard library: [I/O](docs/stdlib/io.md) | [Types](docs/stdlib/typing.md) | [String](docs/stdlib/string.md) | [List](docs/stdlib/list.md) | [Sys/Time](docs/stdlib/sys.md) | [Math](docs/stdlib/math.md) | [Assert](docs/stdlib/assert.md) | [JSON](docs/stdlib/json.md) | [Net](docs/stdlib/net.md) | [Mixer](docs/stdlib/mixer.md) | [UI](docs/stdlib/ui.md)

## What Arc Gives You

**Bytecode VM.** Your source is parsed into an AST, then compiled into a `Chunk` that holds code, constants, and source positions. The VM is a stack machine that dispatches with computed goto. It is small and quick, and it keeps full position info for nice errors.

**Simple syntax.** Keywords are case insensitive. You can write `VAR`, `var`, or `Var` and it means the same thing. Identifiers are case sensitive. Control flow reads like plain English: `IF`, `ELIF`, `ELSE`, `END`, `WHILE`, `FOR item IN list`, `BREAK`, `CONTINUE`, `TRY ... CATCH`, `RETURN`.

**Functions and classes.** `FN` gives you first class functions with their own locals. `CLASS` gives you simple objects with dynamic fields. Methods take `self` explicitly, so there is no hidden binding.

**Memory that stays out of your way.** The front end uses arenas for parsing and compiling, the runtime uses pools for numbers, strings, functions, and instances. You can run normally, or pass `--cleanup` to free everything on exit for leak checking.

**FFI.** Load a shared library with `dl_open`, look up a symbol with `dl_sym`, call it like a normal function. Works for `.so` on Linux, `.dylib` on macOS, `.dll` on Windows.

**Standard library.** File and stream I/O, strings and lists, math, time, sys, JSON in pure Arc and in fast C, plus optional `net`, `mixer`, `ui`, and `image` when you build the clibs.

**Helpful diagnostics.** Errors show file, line, column, the source line, and a caret that points at the problem. The compiler also warns. For example it will warn if you write `BREAK` outside a loop or `RETURN` outside a function, and it will point at the exact spot. Warnings are yellow, they do not stop compilation, and they go to stdout with file and line info.

**Tooling you expect.** `--version` prints `Arc - version 0.5.0`, `--help` lists flags, `--debug` dumps tokens, AST, and bytecode, `--code` runs a string, and there are flags for float precision, colors, pool size, arena size, last result, and more. Sanitizer builds work with `make debug`.

## Requirements

* `gcc` or `clang`, `make`, `libffi-dev`
* Optional for sanitizers: `ASan` and `UBSan` come with your compiler
* Optional for extra libs: SDL, SDL_mixer, SDL_image where needed

Tested with GCC 16 and Clang 22.

## Build and Install

```bash
# fast dev build, no optimization, with debug info
make            # same as make dev

# sanitizers, good for finding bugs
make debug

# full optimization for shipping
make release

# try it without installing
./arc --version
./arc examples/hello_world.arc
./arc -c 'print("hello from -c")'

# run the test suite
make test       # builds dev, runs 35+ .arc tests and 6 warning checks

# install release binary and stdlib
sudo make install                 # to /usr
sudo make dev-install             # or pick a variant
sudo make debug-install
sudo make release-install
make install PREFIX=$HOME/.local  # to your home

# only the libraries
sudo make install-libs

# remove what you installed
sudo make uninstall

# clean build artifacts
make clean
```

All common flags work:

```
-h, --help
-v, --version
-c, --code <str>
-d, --debug
-p, --float-precision <n>
-n, --disable-colored-formatting
-m, --mempool-size <n>
-A, --arena-block-size <n>
-l, --last-result
-S, --skip-evaluation
-C, --cleanup
```

See `arc --help` and [Getting Started](docs/getting-started.md) for examples and troubleshooting.

## A Quick Example

```arc
VAR list = [1, 2, 3, 4, 5]

FOR item IN list THEN
    IF item == 3 THEN
        CONTINUE
    END
    print("Item:", item)
END
```

More examples live in `examples/`:

* `hello_world.arc` - the basics
* `fibonacci.arc` - recursion and loops
* `classes.arc` - fields, instances, methods with explicit `self`
* `file_io.arc` - reading and writing files
* `list_processing.arc` - list helpers
* `string_processing.arc` - string helpers
* `error_handling.arc` - `TRY` and `CATCH`

## How It Runs

1. **Lexer** turns text into tokens. Keywords are compared case insensitively with a fast SWAR style check.
2. **Parser** is a recursive descent parser that builds an AST. It tracks `Position` for every node.
3. **Compiler** walks the AST and emits bytecode into a `Chunk`. It interns strings, dedups numbers, resolves locals, and patches jumps. Locals live in `Compiler.locals` up to `MAX_LOCALS 256`.
4. **VM** in `src/vm.c` executes the `Chunk` with a value stack (`VM_STACK_MAX 4096`), call frames (`VM_CALL_STACK_MAX 8192`), locals (`VM_LOCALS_MAX 65536`), and a try stack. Numbers are kept as `VAL_INT` or `VAL_FLOAT` directly on the stack, no boxing, so `int64_t` stays intact.

Values are defined in `include/value.h`:

```c
VAL_UNDEF, VAL_NULL, VAL_INT, VAL_FLOAT, VAL_OBJ
```

`VAL_OBJ` points at heap objects like `String`, `List`, `Function`, `Class`, `Instance`, `File`, and `NativeFunction`. Pools back the common ones, arenas back the short lived ones.

## Testing

```bash
make test
```

You will see `PASS` or `FAIL` per file, plus a warning suite. Skipped tests mean you did not build an optional native lib (`image`, `mixer`, `ui`, `c_tools`). `test_clib_net` needs a live Beeceptor endpoint and is expected to fail offline. Warning tests check that `BREAK` or `CONTINUE` outside a loop warns, `RETURN` outside a function warns, and correct uses stay quiet.

Tests live in `tests/*.arc`. Warning checks live in `tests/test_warnings.sh` and are run by `make test`.

## License

GPL-3.0. See `LICENSE`.
