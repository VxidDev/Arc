IMPORT "@stdlib/assert.arc"

# string concatenation
VAR a = "Hello"
VAR b = " World"
VAR c = a + b
assert_true(c == "Hello World", "concatenation")

# string multiplication
VAR d = "Arc"
VAR e = d * 3
assert_true(e == "ArcArcArc", "multiplication")

# equality and mutability
VAR s1 = "test"
VAR s2 = "test"
assert_true(s1 == s2, "initial equality")

s1[0] = "T"
assert_true(s1 != s2, "inequality after mutation")
assert_true(s1 == "Test", "modified value")
assert_true(s2 == "test", "original unchanged")

# edge cases
VAR empty = ""
assert_true(empty + "foo" == "foo", "empty + string")
assert_true(d * 0 == "", "string * 0")
assert_true(d * 1 == "Arc", "string * 1")

# list of strings
VAR list = ["a", "b", "c"]
VAR joined = list[0] + list[1] + list[2]
assert_true(joined == "abc", "joined list elements")

print("test_string_ops.arc passed\n")
