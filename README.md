# Arc - v0.5.0-beta

Arc is a modern, stack-based bytecode VM and programming language designed for simplicity and performance. Source is compiled to custom bytecode and executed on a high-performance VM with computed-goto dispatch, arena/pool memory management, and a rich standard library.

## Documentation

Start with [docs/index.md](docs/index.md) → [Getting Started](docs/getting-started.md) → [Language Reference](docs/syntax.md).

| Guide | Description |
|---|---|
| [Getting Started](docs/getting-started.md) | Build, run, CLI flags, install |
| [Language Reference](docs/syntax.md) | Syntax, literals, control flow, imports |
| [Architecture](docs/architecture.md) | Compiler and VM pipeline |
| [Bytecode Reference](docs/bytecode.md) | Instruction set |
| [Object System](docs/object_system.md) | Values, objects, memory |
| [FFI](docs/ffi.md) | Calling native C libraries |

Standard library docs: [I/O](docs/stdlib/io.md) · [Typing](docs/stdlib/typing.md) · [String](docs/stdlib/string.md) · [List](docs/stdlib/list.md) · [Sys/Time](docs/stdlib/sys.md) · [Math](docs/stdlib/math.md) · [Assert](docs/stdlib/assert.md) · [JSON](docs/stdlib/json.md) · [Net](docs/stdlib/net.md) · [Mixer](docs/stdlib/mixer.md) · [UI](docs/stdlib/ui.md)

## Features

* **Bytecode VM** - AST → Chunk → stack VM with constant interning, jump patching, and labels-as-values dispatch.
* **Clean Syntax** - case-insensitive keywords (`VAR`/`var`/`Var`), expressive control flow (`IF`/`WHILE`/`FOR...IN`/`BREAK`/`CONTINUE`/`TRY...CATCH`).
* **Object-Oriented** - `CLASS` with dynamic fields, `self`-explicit methods, first-class `FN` functions.
* **Memory Management** - arenas for parse/compile phases, pools for runtime objects, precise cleanup via `--cleanup`.
* **FFI** - load and call native `.so`/`.dll` libraries.
* **Standard Library** - I/O, math, string/list, JSON (pure Arc + C), filesystem, sys/time, net, mixer, UI, image.
* **Diagnostics** - position-aware errors and **compiler warnings** (`BREAK`/`CONTINUE` outside loops, `RETURN` outside functions, unknown operators) with source snippet + caret.
* **Tooling** - version flag, debug dump, configurable mempool/arena, colored output, sanitizer builds.

## Requirements

* `gcc` (or `clang`), `make`, `libffi-dev`
* Optional: AddressSanitizer/UBSanitizer for `make debug`

## Build and Install

```bash
# Dev build (default, -O0 -g)
make            # or: make dev

# Debug build (ASan + UBSan)
make debug

# Optimized release (-O3 -flto -march=native)
make release

# Run without installing
./arc --version
./arc examples/hello_world.arc
./arc -c 'print("hello from -c")'

# Tests (builds dev binary, runs 35+ arc tests + 6 warning tests)
make test

# Install (release binary + stdlib to $PREFIX, default /usr)
sudo make install
# or: sudo make dev-install / debug-install / release-install
# custom prefix: make install PREFIX=$HOME/.local

# Install libs only
sudo make install-libs

# Uninstall
sudo make uninstall

# Clean
make clean
```

CLI flags: `-h/--help`, `-v/--version`, `-c/--code`, `-d/--debug`, `-p/--float-precision`, `-n/--disable-colored-formatting`, `-m/--mempool-size`, `-A/--arena-block-size`, `-l/--last-result`, `-S/--skip-evaluation`, `-C/--cleanup`. See `arc --help` and [Getting Started](docs/getting-started.md).

## Example

```arc
VAR list = [1, 2, 3, 4, 5]

FOR item IN list THEN
    IF item == 3 THEN
        CONTINUE
    END
    print("Item:", item)
END
```

More in [`examples/`](examples/): `hello_world.arc`, `fibonacci.arc`, `classes.arc`, `file_io.arc`, `list_processing.arc`, `string_processing.arc`, `error_handling.arc`.

## Testing

```bash
make test   # PASS/FAIL per file + warning suite summary
```

Tests live in `tests/*.arc`; warning coverage in `tests/test_warnings.sh` (invoked by `make test`). Skipped tests require optional native libs (`image`/`mixer`/`ui`/`c_tools`); `test_clib_net` requires a live Beeceptor endpoint.

## License

GPL-3.0
