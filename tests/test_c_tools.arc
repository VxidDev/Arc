import "__c_tools"

var lib = dl_open("./lib.so", 1)
var func = dl_sym(lib, "add", 2, false)

print(func(1, 3))

dl_close(lib)
