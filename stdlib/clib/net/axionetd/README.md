# AxionetD

AxionetD is a high-performance, lightweight HTTP server framework written in C. Built from scratch using raw sockets and `epoll`, it provides an efficient way to create HTTP servers with custom routing, multi-threading support, and minimal memory overhead.

## Features

- **Efficient I/O:** Uses `epoll` for scalable event-driven networking.
- **Multi-threading:** Built-in thread pool for handling requests concurrently.
- **Memory Management:** Integrated memory pooling for fast allocations and reduced fragmentation.
- **JSON Support:** Built-in JSON parsing and handling powered by `yyjson`.
- **Routing:** Simple and flexible routing system supporting specific HTTP methods or wildcards.
- **Python Wrapper:** Full-featured Python bindings using `ctypes` for easy integration.
- **Graceful Shutdown:** Support for SIGINT (Ctrl+C) to safely stop the server.

## Project Structure

```text
axionetd/
├── include/           # Header files (Public API)
├── src/               # Core C implementation
├── python-wrapper/    # Python bindings and examples
├── base.c             # C example entry point
├── makefile           # Build system
└── LICENSE            # GPL-3.0 License
```

## Getting Started

### Prerequisites

- GCC (or any C11 compatible compiler)
- `make`
- `yyjson` library installed on your system

### Building and Installing

To build the shared library:

```bash
make
```

To install the library and headers globally:

```bash
sudo make install
```

## Example Usage (C)

```c
#include <axionetd/axionetd.h>
#include <axionetd/http.h>
#include <stdio.h>

void root_handler(AxioRequest* request, AxioResponse* response, MemoryPool* responsePool) {
    HTMLResponse(response, "<h1>Hello from AxionetD!</h1>", 200, NULL, 0, responsePool);
}

int main(void) {
    // host, port, backlog, workers, enable_logging
    Axionet* server = initServer("0.0.0.0", 8000, 8192, 4, true);

    if (!server) {
        fprintf(stderr, "Failed to create server.\n");
        return 1;
    }

    AxioRoute route;
    // server, path, methods (NULL for all), methods_count, handler, route_struct, threaded
    addRoute(server, "/", NULL, 0, root_handler, &route, false);

    printf("Server starting on http://0.0.0.0:8000\n");
    startServer(server);

    return 0;
}
```

## Example Usage (Python)

AxionetD provides a clean Pythonic interface:

```python
from axionet import AxionetInstance, cstr_to_py

# host, port, backlog, workers, logging
server = AxionetInstance("127.0.0.1", 8080, 10, 4, True)
server.init_server()

@server.route("/", ["GET"])
def root(req, resp, memory_pool):
    return "<h1>Hello from Python!</h1>", 200, [("Content-Type", "text/html")]

if __name__ == "__main__":
    server.start_server()
```

## Performance & Design

- **Epoll-based:** Optimized for Linux to handle many simultaneous connections with low latency.
- **Thread-safe:** Routes can be marked as `threaded` to be processed by the worker pool.
- **Zero-allocation path:** Where possible, the server reuses buffers from the memory pool to avoid expensive `malloc`/`free` calls during request handling.

## License

GPL-3.0 license
