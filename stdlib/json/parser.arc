import "./lexer.arc"

fn json_parse_value(tokens, idx) then
  if (tokens[idx].type == JSON_TOK_STRING) or (tokens[idx].type == JSON_TOK_INT) then 
    return [idx + 1, tokens[idx].content] 
  end

  if tokens[idx].type == JSON_TOK_LCURLBRACK then 
    return json_parse_object(tokens, idx)
  end

  RuntimeError("Invalid JSON value.")
end 

fn json_parse_object(tokens, idx) then 
  idx = idx + 1 # skip '{'
  var len = len_of(tokens)
  
  var objects = []

  while tokens[idx].type != JSON_TOK_RCURLBRACK then 
    if not (idx < len) then 
      RuntimeError("Expected '}'.")
    end 

    if tokens[idx].type != JSON_TOK_STRING then 
      RuntimeError("Key must be a string.")
    end

    res = json_parse_value(tokens, idx)

    idx = res[0]
    key = res[1]

    if not (idx < len) or (tokens[idx].type != JSON_TOK_COLON) then 
      RuntimeError("Expected ':'.")
    end 

    idx = idx + 1 # skip colon 

    if not (idx < len) then 
      RuntimeError("Expected value.")
    end 

    res = json_parse_value(tokens, idx)
    
    idx = res[0]
    val = res[1]

    objects = append_list(objects, [key, val])

    if (idx < len) and (tokens[idx].type == JSON_TOK_COMMA) then 
      idx = idx + 1
    else 
      break 
    end 
  end 

  return [idx, objects]
end 
