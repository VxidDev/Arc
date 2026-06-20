import "../stdlib/json/lexer.arc"
import "__sys"

var json_str = "{\"test\": 1}"

print("json string:", json_str, "\n")

var tokens = json_lexer(json_str)

for tok in tokens then 
  repr = tok.repr(tok)

  write(1, repr, len_of(repr))
  write(1, " ", 1)
end 

print("\ntest_stdlib_json.arc passed")
