IMPORT "@stdlib/assert.arc"

# string equality edge cases
assert_true("" == "", "empty == empty")
assert_true("a" != "", "non-empty != empty")
assert_true("abc" == "abc", "same content equal")

# string concatenation edge cases
assert_true("" + "" == "", "empty + empty")
assert_true("a" + "" == "a", "non-empty + empty")
assert_true("" + "b" == "b", "empty + non-empty")

# string multiplication edge cases
assert_true("" * 5 == "", "empty * n")
assert_true("x" * 1 == "x", "char * 1")
assert_true("ab" * 2 == "abab", "multi-char * 2")

# string indexing
VAR s = "hello"
assert_true(s[0] == "h", "index 0")
assert_true(s[4] == "o", "last index")
s[0] = "H"
assert_true(s == "Hello", "mutation via index")

# string in list
VAR words = ["foo", "bar", "baz"]
assert_true(words[0] == "foo", "list string 0")
assert_true(words[2] == "baz", "list string 2")

# string comparison operators
# note: < > <= >= on strings return null in Arc
assert_true("abc" == "abc", "string self-equality")
assert_true("abc" != "def", "string inequality")

# string with special characters
assert_true(len_of("\n") == 1, "newline length")
assert_true(len_of("\t") == 1, "tab length")
assert_true(len_of(" ") == 1, "space length")

# long string
VAR long = ""
VAR i = 0
WHILE i < 100 THEN
  long = long + "x"
  i = i + 1
END
assert_eq(len_of(long), 100, "built string length 100")
assert_true(long == "x" * 100, "built string equals multiplied")

print("test_string_edge.arc passed\n")
