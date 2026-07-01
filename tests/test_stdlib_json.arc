import "@stdlib/json/json.arc"

import "__sys"

var json_str = "{\"test\": 1}"

print("json string:", json_str, "\n\n--- Manual Parsing (json_lexer, json_parse_value) --- \n")

var tokens = json_lexer(json_str)

for tok in tokens then 
  repr = tok.repr(tok)

  write(1, repr, len_of(repr))
  write(1, " ", 1)
end 

if len_of(tokens) != 5 then
  print("\n\nERROR: len_of(tokens) != 5")
  exit(1)
end 

var map = json_parse_value(tokens, 0)[1]

print("\n")
print(map)

if len_of(map) != 1 then 
  print("\n\nERROR: len_of(map) != 1")
  exit(1)
end 

print("\nManual parsing test passed.\n\n--- to_json Parsing ---\n")
map = to_json(json_str)

print(map)

if len_of(map) != 1 then 
  print("ERROR: len_of(map) != 1")
end 

print("\nto_json Parsing test passed.\n\n--- json_get_value test ---\n")

var val = json_get_value(map, "test")

if val == null then 
  print("ERROR: val == null")
  exit(1)
end 

if val != 1 then 
  print("ERROR: val != 1")
  exit(1)
end

print("test's value:", val)
print("\n--- json_get_value test passed ---")

print("\ntest_stdlib_json.arc passed")
