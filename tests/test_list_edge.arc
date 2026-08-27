IMPORT "@stdlib/assert.arc"

# list of lists (matrix)
VAR matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
assert_eq(matrix[0][0], 1, "matrix[0][0]")
assert_eq(matrix[1][1], 5, "matrix[1][1]")
assert_eq(matrix[2][2], 9, "matrix[2][2]")
assert_eq(matrix[0][2], 3, "matrix[0][2]")
assert_eq(matrix[2][0], 7, "matrix[2][0]")

# build matrix with loops
VAR m = []
VAR r = 0
WHILE r < 3 THEN
  VAR row = []
  VAR c = 0
  WHILE c < 3 THEN
    append_list(row, r * 3 + c)
    c = c + 1
  END
  append_list(m, row)
  r = r + 1
END
assert_eq(m[0][0], 0, "built matrix[0][0]")
assert_eq(m[1][1], 4, "built matrix[1][1]")
assert_eq(m[2][2], 8, "built matrix[2][2]")

# append then pop
VAR lst = []
append_list(lst, 10)
append_list(lst, 20)
append_list(lst, 30)
assert_eq(len_of(lst), 3, "after 3 appends")
pop_list(lst)
assert_eq(len_of(lst), 2, "after 1 pop")
assert_eq(lst[0], 10, "pop keeps first")
assert_eq(lst[1], 20, "pop keeps second")

# list with null elements
VAR with_null = [1, null, 3]
assert_eq(len_of(with_null), 3, "null in list counts")
assert_eq(with_null[0], 1, "before null")
assert_eq(with_null[2], 3, "after null")

# nested empty lists
VAR ne = [[], [[]], []]
assert_eq(len_of(ne), 3, "nested empty list length")
assert_eq(len_of(ne[0]), 0, "first empty")
assert_eq(len_of(ne[1]), 1, "middle has one element")
assert_eq(len_of(ne[1][0]), 0, "innermost empty")

# large list operations
VAR large = []
VAR i = 0
WHILE i < 50 THEN
  append_list(large, i * 2)
  i = i + 1
END
assert_eq(len_of(large), 50, "large list length")
assert_eq(large[0], 0, "large list first")
assert_eq(large[49], 98, "large list last")

# list as function argument
FN sum_list(lst) THEN
  VAR total = 0
  VAR i = 0
  WHILE i < len_of(lst) THEN
    total = total + lst[i]
    i = i + 1
  END
  RETURN total
END

assert_eq(sum_list([1, 2, 3]), 6, "sum_list [1,2,3]")
assert_eq(sum_list([]), 0, "sum_list empty")
assert_eq(sum_list([10]), 10, "sum_list single")
assert_eq(sum_list(range(1, 101)), 5050, "sum_list range 1..100")

# list returned from function
FN make_list(n) THEN
  VAR result = []
  VAR i = 0
  WHILE i < n THEN
    append_list(result, i)
    i = i + 1
  END
  RETURN result
END

VAR made = make_list(5)
assert_eq(len_of(made), 5, "make_list length")
assert_eq(made[0], 0, "make_list first")
assert_eq(made[4], 4, "make_list last")

print("test_list_edge.arc passed\n")
