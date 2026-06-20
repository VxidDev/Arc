import "../properties.arc"

var JSON_TOK_LCURLBRACK = 0
var JSON_TOK_RCURLBRACK = 1
var JSON_TOK_COLON = 2
var JSON_TOK_COMMA = 3
var JSON_TOK_INT = 4 
var JSON_TOK_STRING = 5

class JSONToken 
  var type = -1 
  var content = 0

  fn init(self, type, content) then 
    self.type = type 
    self.content = content 

    return self
  end 

  fn repr(self) then
    var typestr = ""

    if self.type == -1 then 
      typestr = "UNKNOWN"
    elif self.type == JSON_TOK_LCURLBRACK then 
      typestr = "LCURLBRACK"
    elif self.type == JSON_TOK_RCURLBRACK then 
      typestr = "RCURLBRACK"
    elif self.type == JSON_TOK_STRING then 
      typestr = "STRING"
    elif self.type == JSON_TOK_COLON then 
      typestr = "COLON"
    elif self.type == JSON_TOK_INT then 
      typestr = "INT"
    elif self.type == JSON_TOK_COMMA then 
      typestr = "COMMA"
    end 

    return typestr + "(" + to_string(self.content) + ")" 
  end 
end 

fn json_make_string_token(json_str, json_len, idx) then 
  var str = ""

  while idx < json_len then
    c = json_str[idx]
    idx = idx + 1

    if c == "\"" then 
      break
    end 

    str = str + c 
  end 
  
  var token = JSONToken()
  
  return [token.init(token, JSON_TOK_STRING, str), idx]
end

fn json_make_digit_token(json_str, json_len, idx) then 
  var str = ""

  while idx < json_len then
    c = json_str[idx]

    if not is_int(c) then 
      break
    end 

    str = str + c
    idx = idx + 1
  end 
  
  var token = JSONToken()

  return [token.init(token, JSON_TOK_INT, to_int(str)), idx]
end 

fn json_lexer(json_str) then 
  var idx = 0
  var len = len_of(json_str)
  
  var tokens = []

  while idx < len then
    c = json_str[idx]
    idx = idx + 1

    if c == " " then 
      continue
    end
    
    if c == "{" then 
      token = JSONToken()
      append_list(tokens, token.init(token, JSON_TOK_LCURLBRACK, 0))
      continue
    end 

    if c == "}" then 
      token = JSONToken()
      append_list(tokens, token.init(token, JSON_TOK_RCURLBRACK, 0))
      continue
    end

    if c == "\"" then 
      res = json_make_string_token(json_str, len, idx)
      token = res[0]
      idx = res[1]

      append_list(tokens, token)
      continue
    end 

    if c == ":" then 
      token = JSONToken()
      append_list(tokens, token.init(token, JSON_TOK_COLON, 0))
      continue 
    end

    if c == "," then 
      token = JSONToken()
      append_list(tokens, token.init(token, JSON_TOK_COMMA, 0))
      continue 
    end 

    if is_int(c) then 
      res = json_make_digit_token(json_str, len, idx - 1)
      token = res[0]
      idx = res[1]

      append_list(tokens, token)
      continue
    end 

    return RuntimeError("Invalid json character at position: " + to_string(idx))
  end 

  return tokens
end 
