IMPORT "@stdlib/assert.arc"

# empty string operations
VAR es = ""
assert_true(es + "" == "", "empty + empty")
assert_true(es + "x" == "x", "empty + string")
assert_true(es * 5 == "", "empty * 5")
assert_eq(len_of(es), 0, "empty string len")

# empty list operations
VAR el = []
assert_eq(len_of(el), 0, "empty list len")
append_list(el, 1)
assert_eq(len_of(el), 1, "append to empty list")
assert_eq(el[0], 1, "appended element")

# deeply nested expression
VAR deep = (1 + 2) * (3 + 4) - (10 / 2)
assert_eq(deep, 16, "deeply nested expression")

# multiple operator types in one expression
VAR multi = 2 + 3 * 4 - 1
assert_eq(multi, 13, "mixed operators")

# string with spaces
VAR spaced = "  hello  "
assert_eq(len_of(spaced), 9, "string with spaces length")

# large number
VAR big = 999999
assert_eq(big + 1, 1000000, "large number + 1")

# nested function calls
FN inc(x) THEN
  RETURN x + 1
END

assert_eq(inc(inc(inc(0))), 3, "triple nested call")

# function with no explicit return
FN get_null() THEN
END

assert_eq(get_null(), 0, "empty function returns 0")

# list mutation via indexing
VAR mutate = [10, 20, 30]
mutate[1] = 99
assert_eq(mutate[1], 99, "list index mutation")

# string concatenation in loop
VAR built = ""
VAR i = 0
WHILE i < 5 THEN
  built = built + "x"
  i = i + 1
END
assert_true(built == "xxxxx", "string built in loop")

# nested list construction
VAR nested = []
append_list(nested, [1, 2])
append_list(nested, [3, 4])
assert_eq(nested[0][0], 1, "constructed nested [0][0]")
assert_eq(nested[1][1], 4, "constructed nested [1][1]")

# large list
VAR large = []
VAR j = 0
WHILE j < 100 THEN
  append_list(large, j)
  j = j + 1
END
assert_eq(len_of(large), 100, "large list length")
assert_eq(large[99], 99, "large list last element")

print("test_edge_cases.arc passed\n")
