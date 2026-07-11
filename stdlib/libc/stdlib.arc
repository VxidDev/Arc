import "@stdlib/libc/libc.arc"

var __free_ptr = __dl_sym(__libc, "free")

fn free(ptr) then
  c_run(__free_ptr, [C_VOID, C_VOID_PTR], [ptr])
end
