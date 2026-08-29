# FFI - Calling C from Arc

Arc can load a shared library at runtime and call its functions as if they were Arc functions. The bridge is built on `libffi` and on `dlopen`/`dlsym`/`dlclose` in `src/builtIns/ctools.c` and `src/c-bridge.c`.

## What You Can Do

* Load a `.so` on Linux, `.dylib` on macOS, or `.dll` on Windows
* Look up any symbol by name
* Call it with Arc values and get an Arc value back
* Keep the library open as long as you need, then close it

## How It Looks in Arc

You use three builtins that are registered as `__c_tools`:

```arc
import "__c_tools"
import "__lib_tools"
import "__sys"

# pick the right file for the OS
if get_os() == "Linux" then
    var lib = dl_open(stdlib_path() + "/clib/libarcjson.so")
elif get_os() == "MacOS" then
    var lib = dl_open(stdlib_path() + "/clib/libarcjson.dylib")
else
    var lib = dl_open(stdlib_path() + "\\clib\\libarcjson.dll")
end

var myAdd = dl_sym(lib, "my_add", 2, false)
print(myAdd(2, 3))   # calls the C function

dl_close(lib)
```

That is the same pattern the stdlib uses for `json`, `net`, `ui`, `mixer`, and `image`.

## The C Side

A function you want to call from Arc must have this shape:

```c
Object* my_add(Object** args, size_t argCount) {
    // check args[0], args[1] with enforceType, typeofobj, etc.
    // do work, then return a new Object* like initInt(42) or initString(...)
    // on error return (Object*)initProgramError("message")
}
```

Use helpers from `include/object.h` and `include/builtIns/*.h`:

* `enforceType(arg, OBJ_STRING, 1)` for a quick type check
* `initInt`, `initFloat`, `initString`, `noCopyInitString`
* `initProgramError`, `freeObject`, `copyObject`
* `typeofobj(arg0)` for a readable type name

The VM will box `Value` args to `Object**` for you (small buf `16` or `malloc` for more) and unbox the result with `objectToValue`. If you return `OBJ_ERROR`, the VM turns it into a catchable `RuntimeError`.

## Builtins You Get

### `dl_open(path)`
* String `path` to the file.
* Returns an integer handle on success, `ProgramError` on failure.
* On Linux and macOS it uses `RTLD_LAZY | RTLD_GLOBAL`, on Windows it uses `LoadLibrary`.

### `dl_sym(handle, name, paramCount, isVariadic)`
* `handle` from `dl_open`
* `name` is the exact C symbol
* `paramCount` is `requiredArgCount`
* `isVariadic` is `1` or `0`
* Returns a `NativeFunction` object that you can call like any Arc function. If the Arity check fails at call time you get `Function "foo" expects N arguments, got M`.

### `dl_close(handle)`
* Closes the handle, returns `1` or `ProgramError`.

All three are variadic-checked at call time in `src/vm.c:1002` `OP_CALL` for `OBJ_NATIVE_FUNCTION`.

## A Minimal Example

`mylib.c`:
```c
#include "object.h"
Object* my_add(Object** args, size_t argCount) {
    Number* a = (Number*)args[0];
    Number* b = (Number*)args[1];
    int64_t av = a->base.type==OBJ_NUMBER_INT ? a->as.i : (int64_t)a->as.f;
    int64_t bv = b->base.type==OBJ_NUMBER_INT ? b->as.i : (int64_t)b->as.f;
    return (Object*)initInt(av + bv);
}
```

Build it:
```bash
gcc -shared -fPIC -o mylib.so mylib.c -Iinclude
```

Call it:
```arc
import "__c_tools"
var lib = dl_open("./mylib.so")
var add = dl_sym(lib, "my_add", 2, false)
print(add(10, 32))  # 42
dl_close(lib)
```

## Tips

* Use `stdlib_path()` from `__lib_tools` to find `libarc*.so` after `make install` - it points at `$PREFIX/share/arc/lib`.
* Keep `isStatic` in mind. If you return a `String` you created with `initString`, it is heap owned and the VM will free it. If you return a constant interned string, mark `isStatic` so it is not freed twice.
* For long running native code that may call back into Arc, be careful with `SAVE_STATE` and `REFRESH_FRAME` around the call - the VM does this for you in `OP_CALL`.
* On Windows the search path is different. The example in `tests/test_clib_net.arc` shows the three `if get_os()` branches you can copy.

