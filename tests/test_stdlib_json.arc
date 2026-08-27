IMPORT "@stdlib/assert.arc"
IMPORT "@stdlib/json/json.arc"

VAR json_str = "{\"test\": 1}"

# lexer produces correct token count
VAR tokens = json_lexer(json_str)
assert_eq(len_of(tokens), 5, "json lexer token count")

# manual parse produces correct map
VAR map = json_parse_value(tokens, 0)[1]
assert_eq(len_of(map), 1, "manual parse map length")

# to_json parses correctly
VAR map2 = to_json(json_str)
assert_eq(len_of(map2), 1, "to_json map length")

# json_get_value retrieves value
VAR val = json_get_value(map2, "test")
assert_true(val != null, "json_get_value returns non-null")
assert_eq(val, 1, "json_get_value('test') == 1")

print("test_stdlib_json.arc passed\n")
