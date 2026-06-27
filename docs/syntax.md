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
Variables are declared using the `VAR` keyword, though it is optional for subsequent assignments.

```arc
VAR x = 10  # Initial declaration
x = 20      # Re-assignment
VAR x = 30  # Re-declaration (also valid)
```

## Classes

Arc supports a class-based abstraction layer for grouping data and functions. Classes are defined using the `CLASS` keyword.

### Class Syntax
```arc
CLASS name
    VAR field = "abc"

    # 'self' is an explicit parameter and must be passed
    FN func(self, ...) THEN
        ...
    END
END
```

### Instances
Classes are instantiated using function-call syntax:
```arc
VAR instance = name()
```

### Field Access
Instance fields are accessed using dot notation and can be modified dynamically:
```arc
instance.field        # reads field value
instance.field = 123  # modifies field value
```

### Method Calls
Methods are encapsulated functions. Because they are **not implicitly bound** to the instance, you must explicitly pass the instance as the first argument, conventionally named `self`:
```arc
instance.func(instance, ...)
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

## Standard Library

Arc provides a set of built-in functions and modules for core functionality:

- [**I/O**](stdlib/io.md): Input/Output operations.
- [**Types & Properties**](stdlib/typing.md): Inspection and conversion.
- [**String Manipulation**](stdlib/string.md): String operations.
- [**List Manipulation**](stdlib/list.md): List operations.
- [**System & Time**](stdlib/sys.md): System and timing functions.
- [**Math Library**](stdlib/math.md): Mathematical constants and functions.


## Control Flow

### IF Statements
Truthiness in Arc is strictly integer-based. A condition is considered **FALSE** if it evaluates to the integer `0`, and **TRUE** if it evaluates to any non-zero integer. Non-integer types (strings, lists) will cause a `TypeError` if used as conditions.

```arc
IF 1 THEN
    print("This is true")
END

IF 0 THEN
    # This will not execute
ELSE
    print("This is false")
END
```

### WHILE Loops
```arc
VAR i = 0

WHILE i < 5 THEN
    i = i + 1
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
* Logical: `AND`, `OR`, `NOT`

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
