# Language Reference

## Variables
Variables are declared using the `VAR` keyword.

```arc
VAR x = 10
```

To update a variable, you must re-declare it using the same name:

```arc
VAR x = x + 1
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

Arc provides several built-in functions implemented in C for core functionality:

### I/O
- `print(value)`: Prints the value to stdout.
- `get_input()`: Reads input from stdin.
- `open_file(path)`: Opens a file.
- `close_file(file)`: Closes an open file.
- `read_file(file)`: Reads content from a file.

### Type & Properties
- `len_of(value)`: Returns the length of a string or list.
- `typeof(value)`: Returns the type of the value.
- `to_int(value)`: Converts a value to an integer.

### Error Handling
- `RuntimeError(message)`: Raises a runtime error with a message.

## Standard Library (`math.arc`)

`math.arc` provides additional mathematical functions. These can be used by adding `IMPORT "math.arc"` at the top of your script.

### Constants
- `PI`: 3.141592653589793
- `E`: 2.718281828459045
- `TAU`: 6.283185307179586

### Mathematical Functions
- `exp(x)`: Exponential function ($E^x$).
- `logn(x)`: Natural logarithm.
- `log(base, x)`: Logarithm with a specific base.
- `sqrt(x)`: Square root.
- `abs(x)`: Absolute value.
- `sign(x)`: Returns -1 if x < 0, 0 if x == 0, 1 if x > 0.
- `floor(x)`: Rounds down to the nearest integer.
- `fmod(x, y)`: Floating-point modulo.
- `sin(x)`: Sine.
- `cos(x)`: Cosine.
- `tan(x)`: Tangent.
- `max(list)`: Returns the maximum value in a list.
- `min(list)`: Returns the minimum value in a list.

## Control Flow

### IF Statements
```arc
IF x == 5 THEN
    "Five"
ELIF x > 5 THEN
    print("Greater than five")
ELSE
    print("Less than five")
END
```

### WHILE Loops
```arc
VAR i = 0
WHILE i < 5 THEN
    VAR i = i + 1
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
