IMPORT "@stdlib/assert.arc"

# --- List creation ---

VAR empty = []
assert_eq(len_of(empty), 0, "empty list length")

VAR nums = [1, 2, 3, 4, 5]
assert_eq(len_of(nums), 5, "list length")

# --- Indexing ---

assert_eq(nums[0], 1, "index 0")
assert_eq(nums[4], 5, "index last")
assert_eq([10, 20, 30][1], 20, "literal index")

# --- Nested lists ---

VAR nested = [[1, 2], [3, 4], [5, 6]]
assert_eq(nested[0][0], 1, "nested index [0][0]")
assert_eq(nested[1][1], 4, "nested index [1][1]")
assert_eq(nested[2][0], 5, "nested index [2][0]")

# --- Mixed types ---

VAR mixed = [1, "two", 3.0, [4]]
assert_eq(len_of(mixed), 4, "mixed list length")
assert_eq(mixed[0], 1, "mixed: int")
assert_true(mixed[1] == "two", "mixed: string")
assert_eq(mixed[3][0], 4, "mixed: nested list")

# --- Append ---

VAR a = [1, 2]
append_list(a, 3)
assert_eq(len_of(a), 3, "append increases length")
assert_eq(a[2], 3, "append adds to end")

# --- Pop ---

VAR b = [10, 20, 30]
pop_list(b)
assert_eq(len_of(b), 2, "pop decreases length")

# --- Range ---

VAR r = range(0, 5)
assert_eq(len_of(r), 5, "range(0,5) length")
assert_eq(r[0], 0, "range(0,5) first")
assert_eq(r[4], 4, "range(0,5) last")

VAR r2 = range(5, 10)
assert_eq(r2[0], 5, "range(5,10) first")
assert_eq(r2[4], 9, "range(5,10) last")

VAR r3 = range(3, 0)
assert_eq(len_of(r3), 3, "range(3,0) descending length")
assert_eq(r3[0], 3, "range(3,0) first")
assert_eq(r3[2], 1, "range(3,0) last")

# --- Empty range ---

VAR r4 = range(5, 5)
assert_eq(len_of(r4), 0, "range(5,5) empty")

# --- List with repeated elements ---

VAR rep = [0, 0, 0, 0, 0]
assert_eq(len_of(rep), 5, "repeated elements length")

# --- Nested empty lists ---

VAR ne = [[], [], []]
assert_eq(len_of(ne), 3, "empty nested lists length")
assert_eq(len_of(ne[0]), 0, "inner empty list length")

print("test_list_ops.arc passed\n")
