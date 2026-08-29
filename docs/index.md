# Arc Documentation - v0.5.0-beta

Welcome to Arc. This is the map. If you read these in order you will know how to build Arc, how to write it, and how it works inside.

## Core Docs

* **[Getting Started](getting-started.md)** - Build the project, run programs, understand every CLI flag, install and test, and fix common issues.
* **[Language Reference](syntax.md)** - How Arc looks and behaves. Covers case rules, literals, variables, functions, classes, lists, operators, imports, and error handling with `TRY` and `CATCH`.
* **[Architecture](architecture.md)** - The path from source text to bytecode to execution. Lexer, parser, compiler, and VM, plus how memory is organized.
* **[Bytecode Reference](bytecode.md)** - Every opcode the VM understands, what it pops and pushes, and what it does.
* **[Object System](object_system.md)** - What a `Value` is, what heap objects exist, how `String`, `List`, `Function`, `Class`, and `Instance` are laid out, and how they are freed.
* **[FFI](ffi.md)** - How to load a C library at runtime with `dl_open`, `dl_sym`, and `dl_close`, and how to call it safely.

## Standard Library

These are the modules you can `IMPORT` in your programs:

* **[I/O](stdlib/io.md)** - `open_file`, `read_file`, `write_file`, `close_file`, streams.
* **[Types and Properties](stdlib/typing.md)** - `typeof`, `to_int`, `to_string`, `len_of`.
* **[String](stdlib/string.md)** - `split_string`, `char_at`, `append_char`, `string_buffer`, `string_finish`.
* **[List](stdlib/list.md)** - `append_list`, `pop_list`, `range` and friends.
* **[System and Time](stdlib/sys.md)** - `get_os`, `getenv`, `system`, filesystem helpers, `perf_counter`.
* **[Math](stdlib/math.md)** - constants and functions you expect from `math.arc`.
* **[Assert](stdlib/assert.md)** - `assert_eq`, `assert_true` for tests and examples.
* **[JSON](stdlib/json.md)** - pure Arc JSON, plus a fast C version in **[C JSON](stdlib/json_clib.md)**.
* **[Net](stdlib/net.md)** - HTTP and `axionetd` helpers.
* **[Mixer](stdlib/mixer.md)** - audio loading and playback.
* **[UI](stdlib/ui.md)** - windows, events, rects, and rendering.

## How to Use These Docs

1. You want to run something now -> start with [Getting Started](getting-started.md).
2. You want to write Arc -> read [Language Reference](syntax.md) and look at `examples/`.
3. You want to change the VM or compiler -> read [Architecture](architecture.md), then [Bytecode](bytecode.md) and [Object System](object_system.md).
4. You want to call C -> read [FFI](ffi.md).

All docs assume `v0.5.0-beta` - bytecode VM, pools and arenas, warnings with source snippets, and the current flag set. If something in the docs does not match what you see when you run `arc --help`, that is a bug - please file it.

