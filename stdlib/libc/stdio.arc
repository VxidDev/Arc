import "@stdlib/libc/libc.arc"

var __printf_ptr = __dl_sym(__libc, "printf")
var __puts_ptr = __dl_sym(__libc, "puts")
var __getchar_ptr = __dl_sym(__libc, "getchar")
var __getc_ptr = __dl_sym(__libc, "getc")

var stdin = pointer_at(__dl_sym(__libc, "stdin"))
var stdout = pointer_at(__dl_sym(__libc, "stdout"))
var stderr = pointer_at(__dl_sym(__libc, "stderr"))

fn getc(stream) then # stream MUST be pointer, no way to validate it there
  return c_run(__getc_ptr, [C_INT, C_INT], [stream])
end 

fn getchar() then 
  return c_run(__getchar_ptr, [C_INT], [])
end 

fn puts(s) then 
  return c_run(__puts_ptr, [C_INT, C_CHAR_PTR], [s])
end 

fn printf(fmt, values) then 
  signature = [C_INT, C_CHAR_PTR]
  args = [fmt]

  for v in values then
    var t = typeof(v)

    if t == "string" then 
      append_list(signature, C_CHAR_PTR)
    elif t == "int" then 
      append_list(signature, C_INT)
    elif t == "float" then 
      append_list(signature, C_DOUBLE)
    elif t == "list" then 
      append_list(signature, C_VOID_PTR)
    end 

    append_list(args, v)
  end 

  return __printf(fmt, signature, args)
end 

fn __printf(fmt, signature, values) then 
  return c_run(__printf_ptr, signature, values, 1) # 1 = fixed arg
end 
