IMPORT "@stdlib/assert.arc"

# equality

assert_true(1 == 1, "int equality")
assert_true(5 == 5, "int equality 2")
assert_true(1 != 2, "int inequality")
assert_true(1 != 2, "int not equal")
assert_true(5 == 5, "int self-equal")

assert_true(1.5 == 1.5, "float equality")
assert_true(1.5 != 2.5, "float not equal")

assert_true("hello" == "hello", "string equality")
assert_true("hello" != "world", "string not equal")
assert_true("" == "", "empty string equality")

# less than / greater than

assert_true(1 < 2, "int less than")
assert_true(NOT (2 < 1), "int less than false")
assert_true(5 > 3, "int greater than")
assert_true(NOT (3 > 5), "int greater than false")
assert_true(1 <= 1, "int less or equal (equal)")
assert_true(1 <= 2, "int less or equal (less)")
assert_true(2 >= 2, "int greater or equal (equal)")
assert_true(3 >= 1, "int greater or equal (greater)")

# float comparisons

assert_true(1.5 < 2.5, "float less than")
assert_true(3.0 > 2.0, "float greater than")
assert_true(1.5 <= 1.5, "float less or equal")
assert_true(2.0 >= 2.0, "float greater or equal")

# chained comparisons

assert_true(1 < 2 AND 2 < 3, "chained less than")
assert_true(NOT (1 < 2 AND 2 > 3), "chained mixed: false because 2>3 is false")

# self-comparison

assert_true(42 == 42, "self equality int")
assert_true("x" == "x", "self equality string")

print("test_comparisons.arc passed\n")
