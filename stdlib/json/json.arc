import "./lexer.arc"
import "./parser.arc"

import "../properties.arc"

class JSONMap
  var length = 0
  var kv_pairs = []
end

fn to_json(str) then
  tokens = json_lexer(str)
  return json_parse_value(tokens, 0)[1]
end 
