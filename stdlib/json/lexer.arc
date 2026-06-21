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
  var buf = string_buffer()

  while idx < json_len then
    c = json_str[idx]
    idx = idx + 1

    if c == "\"" then 
      break
    end 

    append_char(buf, c) 
  end 
  
  var str = string_finish(buf)
  var token = JSONToken()
  
  return [token.init(token, JSON_TOK_STRING, str), idx]
end

fn json_make_digit_token(json_str, json_len, idx) then 
  var buf = string_buffer()

  while idx < json_len then
    c = json_str[idx]

    if not is_digit(c) then 
      break
    end 

    append_char(buf, c)
    idx = idx + 1
  end 
    
  var str = string_finish(buf)
  var token = JSONToken()

  return [token.init(token, JSON_TOK_INT, to_int(str)), idx]
end 

fn json_lexer(json_str) then 
  var idx = 0
  var len = len_of(json_str)
  
  var tokens = []
  
  var lcurl_tok = JSONToken()
  lcurl_tok.init(lcurl_tok, JSON_TOK_LCURLBRACK, 0)
  
  var rcurl_tok = JSONToken()
  rcurl_tok.init(rcurl_tok, JSON_TOK_RCURLBRACK, 0)
  
  var colon_tok = JSONToken()
  colon_tok.init(colon_tok, JSON_TOK_COLON, 0)
  
  var comma_tok = JSONToken()
  comma_tok.init(comma_tok, JSON_TOK_COMMA, 0)

  while idx < len then
    print(idx)

    c = json_str[idx]
    idx = idx + 1

    if c == " " then 
      continue
    end
    
    if c == "{" then 
      append_list(tokens, lcurl_tok)
      continue
    end 

    if c == "}" then 
      append_list(tokens, rcurl_tok)
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
      append_list(tokens, colon_tok)
      continue 
    end

    if c == "," then 
      append_list(tokens, comma_tok)
      continue 
    end 

    if is_digit(c) then 
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
