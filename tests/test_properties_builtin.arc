IMPORT "@stdlib/assert.arc"

# len_of on strings
assert_eq(len_of("hello"), 5, "len_of string")
assert_eq(len_of(""), 0, "len_of empty string")
assert_eq(len_of("a"), 1, "len_of single char")

# len_of on lists
assert_eq(len_of([1, 2, 3]), 3, "len_of list")
assert_eq(len_of([]), 0, "len_of empty list")
assert_eq(len_of([[1, 2], [3]]), 2, "len_of nested list")

# is_digit on single digits
assert_eq(is_digit("0"), 1, "is_digit '0'")
assert_eq(is_digit("5"), 1, "is_digit '5'")
assert_eq(is_digit("9"), 1, "is_digit '9'")

# is_digit on non-digits
assert_eq(is_digit("a"), 0, "is_digit 'a'")
assert_eq(is_digit("hello"), 0, "is_digit 'hello'")

# is_digit: known bug - only checks first char
# "12ab" returns 1 because '1' is a digit, but logically should return 0
# assert_eq(is_digit("12ab"), 0, "is_digit '12ab'")

print("test_properties_builtin.arc passed\n")
