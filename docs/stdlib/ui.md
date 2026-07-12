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

* **`ui.get_mouse_info(ev)`**
  * **Description:** Populates native mouse position and state details from a given event.
  * **Parameters:**
    * `ev` (Event): The active event instance.

* **`ui.get_key_info(ev)`**
  * **Description:** Extracts keyboard hardware key state info from a given event.
  * **Parameters:**
    * `ev` (Event): The active event instance.

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

#### Methods
* **`Event.cleanup()`**
  * **Description:** Dismantles the event structure and manually frees native memory allocation handles.