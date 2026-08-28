# Getting Started - Arc v0.5.0-beta

## Requirements

* `gcc` (or `clang`), `make`, `libffi-dev`
* Optional: `libffi`, SDL/mixer libs for `stdlib/clib/{mixer,ui,image,json,net}`

## Build

Arc uses a `makefile` with several build targets:

* **dev** (default): `-O0 -g`, no sanitizers.
  ```bash
  make dev
  # or simply
  make
  ```

* **debug**: ASan + UBSan, `-fno-omit-frame-pointer`.
  ```bash
  make debug
  ```

* **release**: `-O3 -flto=auto -march=native -mtune=native` + section GC.
  ```bash
  make release
  ```

* **profile**: `-O2 -g -pg`.
  ```bash
  make profile
  ```

Extra flags: `make dev EXTRA_CFLAGS="-D..." EXTRA_LDFLAGS="-L..."`. Verbose: `make V=1`.

## Run

```bash
arc                 # REPL (if installed) - use ./arc when running from repo
./arc script.arc    # run a file
./arc -c 'print(1+2)'  # execute string
```

### Options

| Flag | Description |
|---|---|
| `-h, --help` | Show help |
| `-v, --version` | Print version (e.g. `Arc - version 0.5.0`) |
| `-c, --code <str>` | Execute code from string |
| `-d, --debug` | Dump tokens, AST, and disassembled bytecode |
| `-p, --float-precision <n>` | Float print precision |
| `-n, --disable-colored-formatting` | Disable ANSI colors |
| `-m, --mempool-size <n>` | Object pool size |
| `-A, --arena-block-size <n>` | Arena block size in KB |
| `-l, --last-result` | Print last expression result |
| `-S, --skip-evaluation` | Compile only (syntax check) |
| `-C, --cleanup` | Free all memory before exit (for leak checkers) |

Examples:

```bash
arc -d script.arc
arc -p 10 -c 'print(1/3)'
arc -n script.arc
arc -m 2048 -A 512 script.arc
arc -S script.arc        # syntax check only
arc -C script.arc        # with full cleanup
arc --version
arc --help
```

Diagnostics: errors show file/line/column + source line + `^` caret in red; **warnings** (yellow) for `BREAK`/`CONTINUE` outside loops, `RETURN` outside functions, and unknown operators - compilation continues.

### Install

```bash
sudo make install              # release binary -> /usr/bin + libs -> /usr/share/arc/lib
sudo make dev-install          # dev binary
sudo make debug-install
sudo make release-install
# custom prefix
make install PREFIX=$HOME/.local DESTDIR=""

# libs only
sudo make install-libs

# cross-compile (MinGW)
make CC=x86_64-w64-mingw32-gcc
```

### Uninstall / Clean

```bash
sudo make uninstall
make clean
```

## Testing

```bash
make test
```

Runs `tests/*.arc` (PASS/FAIL/SKIP per file) and the warning suite `tests/test_warnings.sh`:

* **PASS** - output contains `passed`
* **FAIL** - non-zero exit or `Runtime Error`/`FAIL` in output
* **SKIP** - native libs not built (`clib/image`, `clib/mixer`, `clib/ui`, `c_tools`)

`test_clib_net.arc` requires a live Beeceptor endpoint and is expected to fail offline. Warning tests verify that `BREAK`/`CONTINUE`/`RETURN` outside their valid scopes emit yellow warnings with source context, and produce no warning when used correctly.

## Next Steps

* [Language Reference](syntax.md) - literals, variables, classes, lists, control flow, imports, `TRY...CATCH`
* [Architecture](architecture.md) - lexer → parser → compiler → VM pipeline
* [Bytecode Reference](bytecode.md)
* Examples in `examples/` (`hello_world.arc`, `fibonacci.arc`, `classes.arc`, …)

## Troubleshooting

* **Colors garbled** - use `-n` or pipe through `cat`.
* **Leak check** - run with `-C` and `valgrind`/`ASan` (`make debug`).
* **Missing libs** - `make dev-libs` / `make release-libs` builds `stdlib/clib/{json,net}/build/*.so` etc.; install copies them to `$PREFIX/share/arc/lib/clib/`.
