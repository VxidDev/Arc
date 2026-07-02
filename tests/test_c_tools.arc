import "__c_tools"
import "__time"

var c_sig = c_func_signature(C_INT, C_INT, C_INT) 
print(c_sig)

var lib = dl_open("./lib.so", 1) # requires ./lib.so with add func 
var add = dl_sym(lib, "add", 2, false)

print(add(1, 3))

var __add_ptr = __dl_sym(lib, "c_add")
var __add_sig = c_func_signature(C_INT, C_INT, C_INT)

fn c_add(x, y) then 
  return c_run(__add_ptr, __add_sig, [x, y])
end 

print(c_add(1, 3))

var i = 0
var start = perf_counter()

while i != 1000000 then 
  i = add(i, 1)
end 

print("(add) Time elapsed:", perf_counter() - start, "s")

i = 0
start = perf_counter()

while i != 1000000 then 
  i = c_add(i, 1)
end 

print("(c_add) Time elapsed:", perf_counter() - start, "s")

var __fn_ptr = __dl_sym(lib, "yield_string")
var __fn_sig = c_func_signature(C_CHAR_PTR)

print(string_at(c_run(__fn_ptr, __fn_sig, [])))

dl_close(lib)
