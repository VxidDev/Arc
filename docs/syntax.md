# Language Reference - Arc v0.5.0-beta

This is how Arc looks when you write it, and how it behaves when you run it.

## Case Rules

Arc makes a clear split:

* **Keywords are case insensitive.** `VAR`, `var`, `Var`, and `vAr` all mean the same thing. The same is true for `FN`, `IF`, `WHILE`, `FOR`, `TRY`, `CATCH`, `RETURN`, `CLASS`, `BREAK`, `CONTINUE`, `IMPORT`, `END`, `THEN`, `IN`, and so on.
* **Identifiers are case sensitive.** `myVariable`, `myvariable`, and `MYVARIABLE` are three different names.

```arc
var x = 10
VAR Y = 20

FN printSum(a, b) THEN
    print(a + b)
END

# printsum would fail - it is not the same as printSum
```

## Comments

A comment starts with `#` and goes to the end of the line:

```arc
# this is a comment
VAR x = 10  # this is also a comment
```

## Literals

### Numbers

Arc has two number forms. Both live in a `Value`.

```arc
VAR i = 123
VAR j = -42
VAR f = 3.14159
VAR g = 0.5
```

### Strings

Strings are double quoted. They support the usual escapes like `\n`, `\t`, `\r`, `\"`, `\\`, and `\0`. A single character like `"a"` is just a string of length one:

```arc
VAR s = "Hello, Arc!"
VAR empty = ""
VAR withEscape = "line one\nline two"
```

### Lists

Lists are ordered collections in square brackets. They can hold different types:

```arc
VAR a = [1, 2, "hello", 0]
VAR empty = []
VAR nested = [1, [2, 3], 4]
```

### Null and Booleans

Arc spells booleans as numbers: `1` is true, `0` is false. `NULL` is a distinct value for nothing. There is also an internal `VAL_UNDEF` that you will see as a `NameError` if you read a variable before assignment:

```arc
VAR isActive = 1
VAR isDone = 0
VAR nothing = NULL

IF 1 THEN
    print("true")
END
```

## Variables and Constants

`VAR` declares a mutable variable. `CONSTVAL` and `CONSTREF` declare constants. At runtime the distinction is tracked in the `Compiler.locals` and in `SymbolTable` entries with `isMutable` and `isReference`:

```arc
VAR x = 10      # mutable, reassignable
x = 20          # ok
VAR x = 30      # redeclare, also ok

CONSTVAL y = 10
y = 20          # runtime error: cannot assign to constant "y"

CONSTREF r = someList
# r = other     # also constant, cannot rebind
```

If you write a lowercase keyword like `var y = 10`, it still declares the same thing - keywords are case insensitive.

A bare assignment without `VAR` also works and will create or update a global:

```arc
x = 10
x = x + 1
```

## Functions

Functions are first class values. You define them with `FN`, give them a name, a parameter list, and a body. The body is a block that ends with `END`:

```arc
FN add(a, b) THEN
    RETURN a + b
END

VAR r = add(2, 3)
print(r)  # 5
```

Parameters become locals. A function remembers its own chunk and its `maxLocals`. Inside a function you can use `RETURN`. Outside a function, `RETURN` will warn and do nothing.

Recursion works as you expect:

```arc
FN fib(n) THEN
    IF n <= 1 THEN
        RETURN n
    END
    RETURN fib(n - 1) + fib(n - 2)
END
```

For speed, prefer iterative when `n` is large. `fibIter(1000)` is `0.03ms` while `fib(30)` recursive is `~90ms`. Memoized is also tiny at `0.008ms`.

## Classes and Instances

A `CLASS` groups fields and methods. Fields are just variables inside the class body. Methods are `FN`s that take `self` explicitly - there is no implicit `this`:

```arc
CLASS Dog
    VAR name = "unknown"

    FN bark(self) THEN
        print("woof from " + self.name)
    END
END

VAR d = Dog()
print(d.name)        # unknown
d.name = "Rex"
d.bark(d)            # woof from Rex
```

You create an instance by calling the class like a function: `Dog()`. Field access uses `.` and is compiled to `OP_PROPERTY_ACCESS` and `OP_PROPERTY_SET`. If you read a missing field you get a `NameError` that says the instance has no property.

Instances keep their fields in their own `SymbolTable` and keep a pointer to the `Class`. `to_string` for `class`, `instance`, `function`, `file`, and `list` currently reports “not yet available” and returns a `ProgramError`.

## Indexing

Both strings and lists can be indexed with `[]`. Indices are `int64_t` and must be in range:

```arc
VAR s = "Arc"
print(s[0])   # A as a one-char string
s[0] = "a"    # ok, single char string only

VAR lst = [10, 20, 30]
print(lst[1]) # 20
lst[1] = 99
lst[1] = lst[1]  # also ok
```

Out of range or assigning a multi character string to a string index raises `IndexError`. Assignment to a list index uses `freeObject` correctly and checks `ownsValue` for strings.

## Operators

Arc gives you the familiar set:

* **Arithmetic:** `+` `-` `*` `/` `^` (power) and unary `-` and `+`
* **Comparison:** `==` `!=` `<` `>` `<=` `>=` - each pushes `1` or `0`
* **Logical:** `AND` `OR` `NOT` (all case insensitive)

Comparisons on numbers use `double` or `int64_t` directly on the `Value` stack for speed. `OP_DIV` checks for `0` and raises `ValueError: Division by zero`. String `+` concatenates, string `* int` repeats.

Unknown binary or unary operators do not crash. The compiler emits a yellow warning that names the operator and says the operation will be ignored.

## Control Flow

### IF, ELIF, ELSE

```arc
IF n <= 1 THEN
    RETURN n
ELIF n == 2 THEN
    print("two")
ELSE
    print("other")
END
```

Every branch leaves one value on the stack - the compiler inserts `OP_LOAD_CONST 0` when there is no `ELSE`, and patches jumps with `OP_JUMP_IF_FALSE` and `OP_JUMP`.

### WHILE

```arc
VAR i = 0
WHILE i < 5 THEN
    print(i)
    i = i + 1
END
```

Compiled as `loopStart: condition, JUMP_IF_FALSE exit, body, POP, JUMP loopStart, exit: 1`.

### FOR IN

```arc
VAR lst = [10, 20, 30]
FOR item IN lst THEN
    print(item)
END

FOR ch IN "Arc" THEN
    print(ch)  # each ch is a one-char string
END
```

`FOR` is compiled to `OP_FOR_PREP` (push iterable, length, index) and `OP_FOR_ITER` (check `index < length`, push item, bump index, or clean up and exit). `BREAK` and `CONTINUE` inside a `FOR` correctly clean up the three values `iterable, length, index` before jumping. Outside any loop they warn and do nothing.

### BREAK and CONTINUE

They only work inside `WHILE` or `FOR`. The compiler tracks `LoopInfo` and patches `breaks` and `continues` lists. Using them at top level gives a warning that points at the keyword with a caret.

### TRY and CATCH

```arc
TRY
    RuntimeError("oops")
CATCH e THEN
    print("caught:", e)
END
```

`TRY` compiles to `OP_TRY_PUSH catch`, `body`, `OP_TRY_POP`, `OP_JUMP end`, `catch: STORE_VAR e, POP, handler, end:`. At runtime `vmRun` keeps a `tryStack`. On error it unwinds frames, restores `sp` and `ip`, pushes the error string, and continues at the catch. If no `TRY` is on the stack, the error returns from `vmRun`.

## Imports

```arc
IMPORT "@math.arc"
IMPORT "@stdlib/json/json.arc"
IMPORT "__c_tools"
IMPORT "__sys"
```

A path that starts with `@` is looked up in `$PREFIX/share/arc/lib`. A name like `__c_tools` with no slash loads a native module registered in `stdlibModules`. At runtime `OP_IMPORT` first checks native modules, then resolves the file via `resolveImportPath` and compiles it into a new `CallFrame` that shares the global `SymbolTable`.

Use `--skip-evaluation` if you just want to check that imports and syntax are valid.

## Error Handling and Diagnostics

 Arc reports errors with file, line, column, the source line, and a `^` caret. Colors are used when stdout is a tty, and you can turn them off with `-n`.

 Warnings are for things that are not fatal but probably wrong:

 * `BREAK` outside a loop
 * `CONTINUE` outside a loop
 * `RETURN` outside a function
 * unknown binary or unary operator

Warnings are yellow, go to stdout, and include file, line, column and a snippet like:

```
Warning: 'BREAK' outside a loop
File tests/foo.arc, line 2, column 1

BREAK
^^^^^
```

## Putting It Together

A small program shows most of the above:

```arc
IMPORT "@stdlib/assert.arc"

CLASS Counter
    VAR count = 0
    FN inc(self) THEN
        self.count = self.count + 1
        RETURN self.count
    END
END

VAR c = Counter()
c.inc(c)
print(c.count)  # 1

VAR lst = []
FOR i IN [1, 2, 3] THEN
    lst = append_list(lst, i * 2)
END
print(lst)  # [2, 4, 6]

TRY
    print(lst[10])
CATCH e THEN
    print("out of range:", e)
END
```

Start that file with `arc -d` to see tokens, the AST, and the disassembly, then without `-d` to just run it.

