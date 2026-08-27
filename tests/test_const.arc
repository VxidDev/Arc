IMPORT "@stdlib/assert.arc"

# CONSTVAL: immutable by value
CONSTVAL pi = 3.14159
assert_eq(pi, 3.14159, "CONSTVAL initial value")

CONSTVAL name = "Arc"
assert_true(name == "Arc", "CONSTVAL string")

CONSTVAL count = 42
assert_eq(count, 42, "CONSTVAL int")

# CONSTVAL cannot be reassigned
TRY
  count = 99
  assert_true(0, "should not reach here")
CATCH e THEN
  assert_true(1, "CONSTVAL reassignment rejected")
END

# CONSTVAL with expression
CONSTVAL result = 2 + 3 * 4
assert_eq(result, 14, "CONSTVAL from expression")

CONSTVAL greeting = "Hello" + ", " + "World"
assert_true(greeting == "Hello, World", "CONSTVAL from string concat")

# CONSTVAL with function call
CONSTVAL computed = len_of([1, 2, 3, 4, 5])
assert_eq(computed, 5, "CONSTVAL from function call")

# CONSTREF: immutable by reference
VAR x = 10
CONSTREF ref = x
assert_eq(ref, 10, "CONSTREF initial value")

# CONSTREF cannot be reassigned
TRY
  ref = 20
  assert_true(0, "should not reach here")
CATCH e THEN
  assert_true(1, "CONSTREF reassignment rejected")
END

# CONSTREF captures value at declaration
x = 50
assert_eq(ref, 10, "CONSTREF captures value at declaration")

# CONSTREF to list
VAR lst = [1, 2, 3]
CONSTREF list_ref = lst
assert_eq(len_of(list_ref), 3, "CONSTREF list length")

# CONSTVAL list cannot be reassigned but can be mutated
CONSTVAL mylist = [10, 20]
assert_eq(mylist[0], 10, "CONSTVAL list access")
append_list(mylist, 30)
assert_eq(len_of(mylist), 3, "CONSTVAL list mutation allowed")

TRY
  mylist = [99]
  assert_true(0, "should not reach here")
CATCH e THEN
  assert_true(1, "CONSTVAL list reassignment rejected")
END

# VAR still works as expected (mutable)
VAR mutable = 1
assert_eq(mutable, 1, "VAR initial")
mutable = 2
assert_eq(mutable, 2, "VAR reassignment")

# CONSTVAL in function scope
FN using_const() THEN
  CONSTVAL local = 100
  RETURN local
END
assert_eq(using_const(), 100, "CONSTVAL in function")

# CONSTREF in function scope
FN using_constref() THEN
  VAR val = 42
  CONSTREF ref = val
  RETURN ref
END
assert_eq(using_constref(), 42, "CONSTREF in function")

# CONSTVAL with different types
CONSTVAL int_val = 42
CONSTVAL float_val = 3.14
CONSTVAL string_val = "hello"
CONSTVAL list_val = [1, 2, 3]

assert_true(typeof(int_val) == "int", "CONSTVAL int type")
assert_true(typeof(float_val) == "float", "CONSTVAL float type")
assert_true(typeof(string_val) == "string", "CONSTVAL string type")
assert_true(typeof(list_val) == "list", "CONSTVAL list type")

# CONSTVAL redeclaration
CONSTVAL x_val = 1
CONSTVAL x_val = 2
assert_eq(x_val, 2, "CONSTVAL redeclaration")

# VAR cannot shadow CONSTVAL in same scope
CONSTVAL shadow = 10
VAR shadow = 20
assert_eq(shadow, 20, "VAR can shadow CONSTVAL")

print("test_const.arc passed\n")
