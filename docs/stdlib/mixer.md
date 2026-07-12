# Mixer Library

The `mixer` library provides audio loading and playback capabilities for the Arc programming language, binding to the native `libarcmixer` shared library. It allows developers to initialize the audio subsystem, load sound files, control playback loops, and clean up audio resources.

A global instance named `mixer` is exported by default to serve as the main interface for managing the audio life cycle.

## Global Methods (`mixer`)

* **`mixer.create_mixer()`**
  * **Description:** Creates and automatically initializes a new instance of the `Mixer` class.
  * **Returns:** An initialized `Mixer` object.

* **`mixer.disable_signal_handlers()`**
  * **Description:** Disables internal audio-related native signal handlers.

* **`mixer.quit()`**
  * **Description:** Shuts down the global audio subsystem and releases native audio hardware hooks.

---

## Classes

### Sound
An object representing a loaded audio asset. This class is instantiated internally via `Mixer.load_sound()`.

#### Properties
* `is_initialized` (Boolean): Tracks whether the sound pointers are safely bound.

---

### Mixer
The core class responsible for managing the audio device state, asset loading, and playback execution.

#### Properties
* `is_initialized` (Boolean): Tracks whether the audio device has been successfully initialized.

#### Methods

* **`Mixer.init()`**
  * **Description:** Initializes the native audio device context. 
  * **Throws:** `RuntimeError` if the mixer instance is already initialized.

* **`Mixer.load_sound(path)`**
  * **Description:** Loads an audio file from the specified file system path into memory.
  * **Parameters:**
    * `path` (String): The path to the audio file.
  * **Returns:** A new `Sound` object configured with native memory pointers.
  * **Throws:** `RuntimeError` if the mixer is not initialized.

* **`Mixer.play_sound(sound, loop)`**
  * **Description:** Triggers playback for a previously loaded `Sound` object.
  * **Parameters:**
    * `sound` (Sound): The target audio object to play.
    * `loop` (Boolean/Integer): The loop flag or count determining if the track should repeat.
  * **Throws:** `RuntimeError` if the mixer is not initialized.

* **`Mixer.cleanup()`**
  * **Description:** Destroys the mixer context and releases native memory handles allocated for the audio device.