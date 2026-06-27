# I/O Library

The I/O library provides tools for standard Input/Output operations, file manipulation, and filesystem utilities.

## Built-in Functions

- `print(value1, ...)`: Prints one or more values to stdout.
- `get_input(prompt)`: Prints prompt and reads input from stdin.
- `open_file(path, mode)`: Opens a file with specified mode.
- `close_file(file)`: Closes an open file object.
- `read_file(file)`: Reads content from a file object.
- `write_file(file, string)`: Writes string to a file object.

## File Class (`stdlib/io/file.arc`)

The `File` class provides a higher-level wrapper for file operations.

### Methods
- `open()`: Opens the file.
- `close()`: Closes the file.
- `write(string)`: Writes a string.
- `read()`: Reads the entire content.
- `read_char()`: Reads a single character.
- `seek(offset, whence)`: Seeks in the file.
- `tell()`: Returns the current position.
- `get_size()`: Returns file size.

## Filesystem Utilities (`stdlib/io/fs.arc`)

- `fs_exists(path)`: Returns `true` if file exists.
- `fs_delete(path)`: Deletes a file.
