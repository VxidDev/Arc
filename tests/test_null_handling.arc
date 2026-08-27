IMPORT "@stdlib/assert.arc"

# null equality
VAR n = null
assert_true(n == null, "null == null")
assert_true(n != 1, "null != int")
assert_true(n != "hi", "null != string")
assert_true(n != [], "null != list")

# null in conditional
VAR result = 0
IF null THEN
  result = 1
END
assert_eq(result, 0, "null is falsy")

# non-null is truthy
VAR x = 42
IF x THEN
  result = 1
END
assert_eq(result, 1, "non-zero int is truthy")

# empty string truthiness
VAR s = ""
IF s THEN
  result = 2
END
assert_eq(result, 1, "empty string is truthy (object)")

# string truthiness
VAR s2 = "hello"
IF s2 THEN
  result = 3
END
assert_eq(result, 3, "non-empty string is truthy")

# null in list
VAR lst = [1, null, 3]
assert_eq(len_of(lst), 3, "null in list counts")
assert_eq(lst[0], 1, "list before null")
assert_eq(lst[2], 3, "list after null")

# null passed to function
FN check_null(x) THEN
  IF x == null THEN
    RETURN 1
  END
  RETURN 0
END

assert_eq(check_null(null), 1, "function receives null")
assert_eq(check_null(0), 0, "function receives non-null")

print("test_null_handling.arc passed\n")
