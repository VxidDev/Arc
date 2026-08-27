IMPORT "@stdlib/assert.arc"
IMPORT "__sys"

# --- Integer arithmetic ---

assert_eq(2 + 3, 5, "int addition")
assert_eq(10 - 4, 6, "int subtraction")
assert_eq(3 * 7, 21, "int multiplication")
assert_eq(10 / 2, 5, "int division")
assert_eq(2 ^ 10, 1024, "int power")
assert_eq(0 + 0, 0, "zero addition")
assert_eq(0 * 999, 0, "zero multiplication")

# --- Float arithmetic ---

assert_eq(1.5 + 2.5, 4.0, "float addition")
assert_eq(5.5 - 1.5, 4.0, "float subtraction")
assert_eq(2.5 * 4.0, 10.0, "float multiplication")
assert_eq(10.0 / 4.0, 2.5, "float division")
assert_eq(2.0 ^ 3.0, 8.0, "float power")

# --- Mixed int/float ---

assert_eq(1 + 2.5, 3.5, "int + float")
assert_eq(5.5 - 3, 2.5, "float - int")
assert_eq(4 * 0.5, 2.0, "int * float")
assert_eq(7 / 2.0, 3.5, "int / float")

# --- Negative numbers ---

assert_eq(-5 + 10, 5, "negative + positive")
assert_eq(-3 * -7, 21, "negative * negative")
assert_eq(-10 / -2, 5, "negative / negative")
assert_eq(-2 ^ 3, -8, "negative power")

# --- Operator precedence ---

assert_eq(2 + 3 * 4, 14, "precedence: mul before add")
assert_eq(10 - 2 * 3, 4, "precedence: mul before sub")
assert_eq((2 + 3) * 4, 20, "precedence: parens override")
assert_eq(2 * 3 + 4 * 5, 26, "precedence: multiple ops")

# --- Chained operations ---

assert_eq(1 + 2 + 3 + 4, 10, "chained addition")
assert_eq(2 * 3 * 4, 24, "chained multiplication")
assert_eq(100 - 10 - 20 - 30, 40, "chained subtraction")

# --- Edge cases ---

assert_eq(1 / 1, 1, "identity division")
assert_eq(100 ^ 0, 1, "anything ^ 0 = 1")
assert_eq(1 ^ 100, 1, "1 ^ anything = 1")

print("test_arithmetic.arc passed\n")
