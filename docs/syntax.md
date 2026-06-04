# Language Reference

## Language Case Sensitivity

Arc distinguishes between keywords and identifiers (variable and function names) regarding case sensitivity:

- **Keywords are Case-Insensitive**: Keywords such as `VAR`, `FN`, `IF`, `WHILE`, `TRY`, and `RETURN` can be written in any case (e.g., `var`, `Var`, `VAR` are all valid and equivalent).
- **Identifiers are Case-Sensitive**: Variable and function names are case-sensitive. `myVariable`, `myvariable`, and `MYVARIABLE` are considered distinct identifiers.

```arc
var x = 10 # 'var' is recognized as 'VAR'
VAR Y = 20 # 'VAR' is also recognized

FN printSum(a, b) THEN
    print(a + b)
END

# This would cause an error because 'printsum' (lowercase) is not defined
# printsum(x, Y) 
```

## Literals

Arc supports various literal values for representing data.

### Numbers

Numbers can be integers or floating-point values.

```arc
VAR integer_num = 123
VAR float_num = 3.14159
```

### Booleans

Arc represents boolean truthiness using numerical values: `0` for false and `1` for true. Expressions that evaluate to true or false will result in these integer values.

```arc
VAR is_active = 1  # Represents TRUE
VAR is_done = 0    # Represents FALSE

IF 1 THEN # This condition is true
    print("This will execute")
END
```

### Strings

Strings are sequences of characters enclosed in double quotes.

```arc
VAR greeting = "Hello, Arc!"
VAR empty_string = ""
```

## Variables
Variables are declared using the `VAR` keyword.

```arc
VAR x = 10
```

To update a variable, you must re-declare it using the same name:

```arc
x = x + 1
```

## Functions
Functions are defined using the `FN` keyword. A `RETURN` keyword can be used to return a value from a function.

```arc
FN add(a, b) THEN
    RETURN a + b
END

VAR result = add(5, 3)
```

## Lists

Lists are ordered collections of items, enclosed in square brackets `[]`.

```arc
VAR my_list = [1, 2, "hello", 0]
VAR empty_list = []
```

You can access elements by their index (starting from 0).

```arc
VAR first_element = my_list[0] // 1
VAR last_element = my_list[3]  // 0
```

List-related built-in functions like `len_of`, `max`, and `min` (math.arc) can be used to query properties or perform operations on lists.

## Built-in Functions

Arc provides several built-in functions implemented in C for core functionality:

### I/O
- `print(value1, ...)`: Prints one or more values to stdout. Values are converted to strings and separated by spaces. A newline is automatically appended. 
- `get_input(prompt)`: Prints prompt and reads input from stdin.
- `open_file(path, mode)`: Opens a file with specified mode (e.g., "r", "w").
- `close_file(file)`: Closes an open file object.
- `read_file(file)`: Reads content from a file object.
- `write_file(file, string)`: Writes string to a file object.

### Type & Properties
- `len_of(value)`: Returns the length of a string or list.
- `typeof(value)`: Returns the type of the value (e.g., "number", "string", "list", "function").
- `to_int(value)`: Converts a value to an integer.

### String Manipulation
- `split_string(string, delimiter)`: Splits a string into a list of strings based on the delimiter.

### List Manipulation
- `append_list(list, value)`: Appends a value to the end of a list.
- `range(start, end)`: Generates a list of numbers from `start` to `end` (exclusive).

### Error Handling
- `RuntimeError(message)`: Raises a runtime error with a message.

## Command Line Arguments

When running a script, Arc exposes command-line arguments through two global variables:

- `argc`: The number of command-line arguments (including the script name).
- `argv`: A list containing the command-line arguments.

```arc
print("Argument count: ", argc)

VAR i = 0

WHILE i < argc THEN
    print("Arg", i, ": " + argv[i])
    i = i + 1
END
```

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
    print("Five")
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

### FOR Loops
Arc supports iterating over lists and strings using the `FOR...IN` syntax.

```arc
VAR my_list = [10, 20, 30]

FOR item IN my_list THEN
    print(item)
END

# Iterating over a string
FOR char IN "Arc" THEN
    print(char)
END
```

### Loop Control Statements


Arc provides `BREAK` and `CONTINUE` keywords to alter the flow of loops.

*   **BREAK**: Terminates the innermost loop immediately.
    ```arc
    VAR i = 0

    WHILE i < 10 THEN
        i = i + 1

        IF i == 5 THEN
            BREAK # Exit the loop when i is 5
        END
    END

    print(i) # Output will be 5
    ```

*   **CONTINUE**: Skips the rest of the current iteration of the innermost loop and proceeds to the next iteration.

    ```arc
    VAR i = 0

    WHILE i < 5 THEN
        i = i + 1
        IF i == 3 THEN
            CONTINUE # Skip printing when i is 3
        END
        print(i)
    END
    # Output will be:
    # 1
    # 2
    # 4
    # 5
    ```

## Operators
* Arithmetic: `+`, `-`, `*`, `/`, `^`
* Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
* Logical: `AND`, `OR`

## Importing Files
```arc
IMPORT "math.arc"
```

## Error Handling

Arc provides a `TRY...CATCH` mechanism for handling runtime errors gracefully.

```arc
TRY
    # Code that might cause an error
    # For example, a built-in function that can raise RuntimeError
    RuntimeError("Something went wrong!")
CATCH e THEN
    # This block executes if an error occurs in the TRY block
    # 'e' will contain the error details string
    print("Caught an error:", e)
END
```

The `TRY` block encloses the code that might throw an error. If an error occurs within the `TRY` block, execution immediately jumps to the `CATCH` block. The error object is then bound to the identifier specified after `CATCH` (e.g., `e` in the example), which can then be used within the `THEN` block. If no error occurs, the `CATCH...THEN` block is skipped.

## Comments

Comments are used to add explanations or prevent execution of code. They are denoted by a hash symbol `#` and extend to the end of the line.

```arc
# This is a single-line comment
VAR x = 10 # This is an end-of-line comment
```
