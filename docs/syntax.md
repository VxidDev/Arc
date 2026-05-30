# Language Reference

## Variables
Variables are declared using the `VAR` keyword.

```arc
VAR x = 10
```

## Functions
Functions are defined using the `FN` keyword. A `RETURN` keyword can be used to return a value from a function.

```arc
FN add(a, b) THEN
    RETURN a + b
END

VAR result = add(5, 3)
```

## Built-in Functions

Arc includes several built-in functions for common tasks:

### I/O
- `print(value)`: Prints the value to stdout.
- `get_input()`: Reads input from stdin.
- `open_file(path)`: Opens a file.
- `close_file(file)`: Closes an open file.
- `read_file(file)`: Reads content from a file.

### Math
- `truncate(value)`: Truncates a number.
- `floor(value)`: Returns the floor of a number.

### Type & Properties
- `len_of(value)`: Returns the length of a string or list.
- `typeof(value)`: Returns the type of the value.
- `to_int(value)`: Converts a value to an integer.

### Error Handling
- `RuntimeError(message)`: Raises a runtime error with a message.

## Control Flow

### IF Statements
```arc
IF x == 5 THEN
    "Five"
ELIF x > 5 THEN
    "Greater than five"
ELSE
    "Less than five"
END
```

### WHILE Loops
```arc
VAR i = 0
WHILE i < 5 THEN
    i = i + 1
END
```

## Operators
* Arithmetic: `+`, `-`, `*`, `/`, `^`
* Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
* Logical: `AND`, `OR`

## Importing Files
```arc
IMPORT "math.arc"
```
