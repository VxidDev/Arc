IMPORT "@stdlib/assert.arc"

# typeof returns type name
assert_true(typeof(42) == "int", "typeof int")
assert_true(typeof(3.14) == "float", "typeof float")
assert_true(typeof("hello") == "string", "typeof string")
assert_true(typeof([1, 2]) == "list", "typeof list")

# to_int from int
assert_eq(to_int(42), 42, "to_int from int")

# to_int from float
assert_eq(to_int(3.9), 3, "to_int truncates float")

# to_int from string
assert_eq(to_int("123"), 123, "to_int from string")
assert_eq(to_int("-5"), -5, "to_int negative string")

# to_string from int
assert_true(to_string(42) == "42", "to_string from int")
assert_true(to_string(0) == "0", "to_string from zero")
assert_true(to_string(-7) == "-7", "to_string from negative")

# to_string from float
VAR fs = to_string(3.14)
assert_true(len_of(fs) > 0, "to_string from float produces non-empty string")

# to_string from string
assert_true(to_string("hello") == "hello", "to_string from string")

# to_string from list
VAR ls = to_string([1, 2, 3])
assert_true(ls == "[1, 2, 3]", "to_string from list")

# typeof on class instance
CLASS Counter
  VAR count = 0
END
VAR c = Counter()
assert_true(typeof(c) == "Counter", "typeof class instance")

print("test_typing.arc passed\n")
