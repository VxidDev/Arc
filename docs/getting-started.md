# Getting Started

## Build

Arc uses a `makefile` with several build targets:

* **dev** (default): Builds with optimizations disabled and debug symbols enabled.
  ```bash
  make dev
  # or simply
  make
  ```

* **debug**: Builds with AddressSanitizer and UndefinedBehaviorSanitizer for deep debugging.
  ```bash
  make debug
  ```

* **release**: Builds with full optimizations (`-O3`, `-flto`, `-march=native`) for maximum performance.
  ```bash
  make release
  ```

## Run

Arc can be run using the `arc` command if installed, or via the local executable.

```bash
arc
# or
./arc
```

### Options

* **Debug mode**:
  ```bash
  arc -d
  # or
  arc --debug
  ```

* **Float precision**:
  ```bash
  arc -p 10
  # or
  arc --float-precision 10
  ```

* **Mempool size**:
  ```bash
  arc -m 2048
  # or
  arc --mempool-size 2048
  ```

* **Print last result**:
  ```bash
  arc -l script.arc
  # or
  arc --last-result script.arc
  ```

* **Help**:
  ```bash
  arc -h
  # or
  arc --help
  ```

* **Execute code**:
  ```bash
  arc -c "5 + 5"
  # or
  arc --code "VAR x = 10"
  ```

* **Disable colors**:
  ```bash
  arc -n
  # or
  arc --disable-colored-formatting
  ```

* **Run a file**:
  ```bash
  arc script.arc
  ```

### Install

The `install` target builds the **release** version and installs it to `/usr/bin` (by default).

```bash
sudo make install
```

You can also install specific versions:
```bash
sudo make dev-install
sudo make debug-install
sudo make release-install
```

### Uninstall

```bash
sudo make uninstall
```
