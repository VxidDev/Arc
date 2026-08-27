IMPORT "@stdlib/assert.arc"

VAR int_val = 123
assert_eq(int_val, 123, "integer variable")

VAR float_val = 1.23
assert_eq(float_val, 1.23, "float variable")

VAR str_val = "string"
assert_true(str_val == "string", "string variable")

VAR list_val = [1, 2.0, "3"]
assert_eq(len_of(list_val), 3, "list variable length")
assert_eq(list_val[0], 1, "list element int")
assert_eq(list_val[1], 2.0, "list element float")
assert_true(list_val[2] == "3", "list element string")

print("test_vars.arc passed\n")
