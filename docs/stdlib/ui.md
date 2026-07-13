# UI Library

The `ui` library provides low-level graphical user interface, window management, and event handling capabilities for the Arc programming language. It serves as a binding to the native `libarcui` shared library, leveraging SDL-like structures to manage rendering stages, geometric shapes, and hardware inputs (keyboard and mouse).

A global instance named `ui` is exported by default to interface with the subsystem.

## Global Interface (`ui`)

### Event Type Constants
These constants are used to evaluate the `type` property of an `Event` instance:
* `ui.MOUSE_BUTTON_DOWN` (0)
* `ui.MOUSE_BUTTON_UP` (1)
* `ui.MOUSE_WHEEL` (2)
* `ui.KEY_DOWN` (3)
* `ui.KEY_UP` (4)
* `ui.QUIT` (5)
* `ui.NOT_MAPPED` (6)

### Methods

* **`ui.create_stage(width, height, window_name)`**
  * **Description:** Initializes a window and a corresponding renderer context wrapped inside a unified `Stage` object.
  * **Parameters:**
    * `width` (Integer): Width of the window in pixels.
    * `height` (Integer): Height of the window in pixels.
    * `window_name` (String): The title displayed on the window bar.
  * **Returns:** A configured `Stage` object.

* **`ui.get_event()`**
  * **Description:** Polls the application event queue for pending user interactions.
  * **Returns:** An `Event` object if an interaction is available; otherwise, returns `null`.

* **`ui.rect(x, y, width, height)`**
  * **Description:** Factory method to construct and initialize a bounded `Rect` object.
  * **Returns:** A newly instantiated `Rect` object.

* **`ui.delay(ms)`**
  * **Description:** Pauses the execution thread for a designated duration.
  * **Parameters:**
    * `ms` (Integer): The delay duration in milliseconds.
---

## Classes

### Rect
A class representing a 2D rectangular boundary. When initialized with borders, it recursively maintains sub-rectangles for its four structural edges (`left`, `right`, `top`, `bottom`) to aid layout collision.

#### Properties
* `x`, `y` (Float/Int): Positional coordinates of the top-left corner.
* `width`, `height` (Float/Int): Dimensions of the rectangle.
* `thickness` (Float): Border thickness (defaults to 5.0).
* `has_border` (Boolean): Flag indicating if edge sub-rectangles are active.

#### Methods
* **`Rect.set_pos(x, y)`**
  * **Description:** Teleports the primary rectangle and recalculates its bounding edge positions instantly.
* **`Rect.collides_with(rect)`**
  * **Description:** Determines if the top-left point of the target rectangle falls within the current rectangle's bounds.
  * **Returns:** Boolean.

---

### Stage
Acts as the central orchestration context managing the viewport window and graphics pipeline.

#### Methods
* **`Stage.clear()`**
  * **Description:** Wipes the renderer canvas clean.
* **`Stage.present()`**
  * **Description:** Updates the display surface with the latest drawn compositions.
* **`Stage.fill(color)`**
  * **Description:** Fills the background canvas with a specified RGBA color profile.
  * **Parameters:**
    * `color` (List): An array containing 4 integers representing `[R, G, B, A]`.
* **`Stage.draw_rect(rect, color)`**
  * **Description:** Renders a filled geometric `Rect` structure on the screen using the target color array.
  * **Parameters:**
    * `rect` (Rect): The target rectangle bounds.
    * `color` (List): An array containing 4 integers `[R, G, B, A]`.
* **`Stage.set_window_resizable(is_resizable)`**
  * **Description:** Toggles the manual window resizing trait dynamically. Accepts integer states `1` (true) or `0` (false).

---

### Event
A utility container wrapping a polled system or hardware message handle.

#### Properties
* `type` (Integer): Resolved mapping corresponding to the `ui` event type constants.
* `key` (String | null): Scancode converted to readable format, for example "Escape". Accessible only when type is KEY_DOWN or KEY_UP, otherwise null.
* `down` (Boolean | null): true on *_DOWN and false on *_UP events, otherwise null.
* `scancode` (Integer | null): Integer value representing key's scancode. Accessible only when type is KEY_DOWN or KEY_UP, otherwise null.
* `mouse_pos_x` (Float | null): Float value representing mouse's position on x axis. Accessible only when type is MOUSE_BUTTON_DOWN or MOUSE_BUTTON_UP, otherwise null.
* `mouse_pos_y` (Float | null): Float value representing mouse's position on y axis. Accessible only when type is MOUSE_BUTTON_DOWN or MOUSE_BUTTON_UP, otherwise null.
* `clicked_button` (Integer | null): Integer value representing currently clicked mouse button. 0 on left and 1 on right mouse button. Accessible only when type is MOUSE_BUTTON_DOWN or MOUSE_BUTTON_UP, otherwise null.
