# Net Library

The `net` library provides built-in networking capabilities for the Arc programming language. It allows the creation of HTTP servers and provides utilities to make basic HTTP client requests by binding to the native `libarcnet` shared library.

## Classes

### Net
A utility class used for handling basic client-side HTTP requests.

#### Methods

* **`Net.init()`**
  * **Description:** Initializes the internal network request system. This must be called before performing any network operations.
  * **Returns:** Initialization status.

* **`Net.deinit()`**
  * **Description:** Shuts down the network request system and cleans up allocated resources.
  * **Returns:** Deinitialization status.

* **`Net.get(url)`**
  * **Description:** Sends a standard HTTP GET request to the specified URL.
  * **Parameters:**
    * `url` (String): The target web address or API endpoint.
  * **Returns:** The response data from the server.

---

### HTTPServer
A class that provides an interface to configure, spin up, and manage a local HTTP server.

#### Properties

* `host` (String): The IP address or hostname the server binds to (e.g., `"127.0.0.1"`).
* `port` (Integer): The network port the server listens on (e.g., `8080`).
* `backlog` (Integer): The maximum length of the queue for pending connections.
* `logging` (Boolean): Toggles whether the server outputs activity logs to the console.
* `is_initialized` (Boolean): Tracks the server's current initialization state.

#### Methods

* **`HTTPServer.init(host, port, backlog, logging)`**
  * **Description:** Configures and prepares the HTTP server instance with the specified binding configurations.
  * **Parameters:**
    * `host` (String)
    * `port` (Integer)
    * `backlog` (Integer)
    * `logging` (Boolean)

* **`HTTPServer.start()`**
  * **Description:** Launches the HTTP server to start listening for incoming requests. Note that the server runs synchronously and can currently only be stopped via `CTRL + C`.
  * **Throws:** `RuntimeError` if called before the server is successfully initialized.

* **`HTTPServer.destroy()`**
  * **Description:** Performs resource cleanup for the server instance. 
  * **Throws:** `RuntimeError` if the server is not initialized.