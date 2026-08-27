IMPORT "@stdlib/assert.arc"

# split string by delimiter
VAR parts = split_string("hello world foo", " ")
assert_eq(len_of(parts), 3, "split into 3 parts")
assert_true(parts[0] == "hello", "split first")
assert_true(parts[1] == "world", "split middle")
assert_true(parts[2] == "foo", "split last")

# split with no match
VAR no_match = split_string("abc", ",")
assert_eq(len_of(no_match), 1, "no delimiter match = 1 part")
assert_true(no_match[0] == "abc", "no match preserves original")

# split single char delimiter
VAR csv = split_string("a,b,c", ",")
assert_eq(len_of(csv), 3, "csv split")

# char_at returns int code
VAR code = char_at("hello", 0)
assert_eq(code, 104, "char_at('hello', 0) = 'h'")

VAR code2 = char_at("hello", 4)
assert_eq(code2, 111, "char_at('hello', 4) = 'o'")

# char_to_string converts code to string
VAR ch = char_to_string(65)
assert_true(ch == "A", "char_to_string(65) = 'A'")

VAR ch2 = char_to_string(97)
assert_true(ch2 == "a", "char_to_string(97) = 'a'")

# string buffer build and finish
VAR buf = string_buffer()
append_char(buf, "H")
append_char(buf, "i")
VAR result = string_finish(buf)
assert_true(result == "Hi", "string buffer build 'Hi'")

# string concatenation
VAR s1 = "foo"
VAR s2 = "bar"
assert_true(s1 + s2 == "foobar", "string concat")

# string multiplication
VAR s3 = "ab"
assert_true(s3 * 3 == "ababab", "string multiply")

# string length
assert_eq(len_of("hello"), 5, "len_of string")
assert_eq(len_of(""), 0, "len_of empty string")

print("test_string_builtins.arc passed\n")
