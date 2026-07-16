import "__c_tools"
import "__time"
import "__sys"

var c_sig = c_func_signature(C_INT, C_INT, C_INT) 
print(c_sig)

if get_os() == "Linux" then
  var lib = dl_open("./lib.so") # requires ./lib.so 
elif get_os() == "MacOS" then
  var lib = dl_open("./lib.dylib") # requires ./lib.dylib
elif get_os() == "Windows" then
  var lib = dl_open("./lib.dll") # requires ./lib.dll
else
  RuntimeError("Unknown OS.")
end

var add = dl_sym(lib, "add", 2, false)

var res = add(1, 3)

if res != 4 then
  RuntimeError("Invalid add(1, 3) result.")
end

var __add_ptr = __dl_sym(lib, "c_add")
var __add_sig = c_func_signature(C_INT, C_INT, C_INT)

fn c_add(x, y) then 
  return c_run(__add_ptr, __add_sig, [x, y])
end 

res = c_add(1, 3)

if res != 4 then
  RuntimeError("Invalid c_add(1, 3) result.")
end

var i = 0
var start = perf_counter()

while i != 1000 then 
  i = add(i, 1)
end 

print("(add) Time elapsed:", perf_counter() - start, "s")

i = 0
start = perf_counter()

while i != 1000 then 
  i = c_add(i, 1)
end 

print("(c_add) Time elapsed:", perf_counter() - start, "s")

var __fn_ptr = __dl_sym(lib, "yield_string")
var __fn_sig = c_func_signature(C_CHAR_PTR)

res = string_at(c_run(__fn_ptr, __fn_sig, []))

if res != "hello" then
  RuntimeError("Invalid string_at() result.")
end

__fn_ptr = __dl_sym(lib, "yield_int")
__fn_sig = c_func_signature(C_INT_PTR)

res = int_at(c_run(__fn_ptr, __fn_sig, []))

if res != 5 then
  RuntimeError("Invalid int_at() result.")
end

var run_fn = dl_sym(lib, "run_fn", 2, false)

fn greet(name) then 
  print("Hello, " + name + "!")
end 

run_fn(greet, ["World"]) # must print "Hello, World!"

dl_close(lib)
