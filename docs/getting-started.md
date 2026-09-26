# Getting Started - Arc v0.5.0-beta

This guide gets you from zero to running Arc, whether you want to try a file, hack on the VM, or install it for daily use.

## What You Need

* `gcc` or `clang`
* `make`
* `libffi-dev` (for FFI)

Optional:
* SDL, SDL_image, SDL_mixer if you want to build `image`, `ui`, `mixer`
* The C JSON and net libs build by default as part of `make`

We test on GCC 16 and Clang 22 on Linux.

## Build

Arc uses a small `makefile` with a few targets:

**Dev build (default) - fast to compile, easy to debug:**
```bash
make        # same as make dev
# gives you -O0 -g, no sanitizers
```

**Debug build - finds memory bugs:**
```bash
make debug
# gives you -O0 -g + ASan + UBSan + -fno-omit-frame-pointer
```

**Release build - what you ship:**
```bash
make release
# gives you -O3 -flto=auto -march=native -mtune=native + section GC
```

**Profile build - for `gprof`:**
```bash
make profile
# gives you -O2 -g -pg
```

You can add your own flags:
```bash
make dev EXTRA_CFLAGS="-DDEBUG" EXTRA_LDFLAGS="-L/opt/lib"
make V=1   # verbose, shows every compile line
```

## Run

From the repo root, without installing:

```bash
./arc --version          # Arc - version 0.5.0
./arc --help
./arc examples/hello_world.arc
./arc -c 'print(1 + 2)'  # run a string
./arc -d script.arc      # dump tokens, AST, and bytecode
```

If you installed with `make install`, you can just run `arc` instead of `./arc`.

### All Flags

| Flag | What it does |
|---|---|
| `-h, --help` | Show help and exit |
| `-v, --version` | Print version like `Arc - version 0.5.0` |
| `-c, --code <str>` | Run the code in `<str>` instead of a file |
| `-d, --debug` | Dump tokens, AST, and disassembled bytecode before running |
| `-p, --float-precision <n>` | How many digits to print for floats |
| `-n, --disable-colored-formatting` | Turn off ANSI colors |
| `-m, --mempool-size <n>` | Pool size for `Number`, `String`, etc (default 1024) |
| `-A, --arena-block-size <n>` | Arena block size in KB (default 256) |
| `-l, --last-result` | Print the value the program leaves on the stack |
| `-S, --skip-evaluation` | Only parse and compile, do not run (syntax check) |
| `-C, --cleanup` | Free arenas and pools on exit - use with `valgrind` or `ASan` |
| `--` | Everything after this is treated as a script argument, not a flag |

Examples:

```bash
./arc -d script.arc
./arc -p 10 -c 'print(1/3)'
./arc -n script.arc
./arc -m 2048 -A 512 script.arc
./arc -S script.arc        # check syntax only
./arc -C script.arc        # full cleanup for leak checkers
./arc -c 'print(argv)' -- arg1 arg2   # script args after --
```

**Diagnostics:** Errors are red and show file, line, column, the source line, and a `^` caret under the problem. Warnings are yellow, they do not stop the build, and they also show file, line, column and a snippet. You will see warnings for `BREAK` or `CONTINUE` outside a loop, `RETURN` outside a function, and for unknown operators.

## Install

```bash
sudo make install              # release binary to /usr/bin, libs to /usr/share/arc/lib
sudo make dev-install          # dev binary
sudo make debug-install        # debug binary
sudo make release-install      # release binary

# to your home directory
make install PREFIX=$HOME/.local

# cross compile with MinGW
make CC=x86_64-w64-mingw32-gcc

# only rebuild the C libs (json, net)
make dev-libs
make release-libs
sudo make install-libs         # just the libs
```

Uninstall and clean:

```bash
sudo make uninstall
make clean
```

The binary needs `libffi` at runtime. The installed stdlib lives in `$PREFIX/share/arc/lib`. When you write `IMPORT "@math.arc"` or `IMPORT "@stdlib/json/json.arc"`, that is where it looks.

## Testing

```bash
make test
```

This builds `dev` if needed and then runs two things:

1. Every `tests/*.arc` file - prints `PASS` in green, `FAIL` in red with the error, or `SKIP` in yellow.
   * `PASS` means the file printed `passed`.
   * `FAIL` means non-zero exit or it saw `Runtime Error` or `FAIL` in output.
   * `SKIP` means a native lib was not built. This happens for `test_clib_image`, `test_clib_mixer`, `test_clib_ui`, and `test_c_tools`.
   * `test_clib_net` needs a live Beeceptor endpoint. Offline it fails with HTML instead of JSON - that is expected.

2. `tests/test_warnings.sh` - six checks:
   * `BREAK` outside a loop warns, inside does not
   * `CONTINUE` outside a loop warns, inside does not
   * `RETURN` outside a function warns, inside does not

The harness caps hanging UI tests with a 3 second timeout. If a test times out it is counted as `SKIP (timeout)` so `make test` does not hang on headless machines.

## Where to Go Next

* [Language Reference](syntax.md) - literals, variables, `CLASS`, `FN`, lists, `IF`, `WHILE`, `FOR`, `DO ... END` blocks, `TRY`/`CATCH`
* [Architecture](architecture.md) - how `src/lexer.c` → `src/parser.c` → `src/compiler.c` → `src/vm.c` fits together
* [Bytecode Reference](bytecode.md) - what each opcode does
* Look at `examples/` - `hello_world.arc`, `fibonacci.arc`, `classes.arc`, `file_io.arc`, `list_processing.arc`, `string_processing.arc`, `error_handling.arc`

## Troubleshooting

**Colors look wrong in my terminal or in a file:**
Use `-n` or pipe through `cat`.

**I want to check for leaks:**
```bash
make debug
./arc -C script.arc
valgrind ./arc -C script.arc
```

**Missing `clib` libs:**
```bash
make dev-libs        # builds stdlib/clib/{json,net}/build/*.so
make release-libs
sudo make install-libs
ls /usr/share/arc/lib/clib/
```

**Verbose build:**
```bash
make V=1
```

