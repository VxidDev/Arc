# C JSON Library (`clib`)

The `clib/json` directory contains a C-based implementation of JSON parsing, designed for high-performance needs, compiled as a shared library (`libarcjson.so`).

## Integration Guide

To use the C JSON library:
1.  Navigate to `stdlib/clib/json/`.
2.  Build the library using the provided makefile:
    ```bash
    make release
    ```
3.  Load `libarcjson.so` within Arc using the [FFI](ffi.md) capabilities (`dl_open`, `dl_sym`).
