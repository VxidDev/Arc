IMPORT "@stdlib/assert.arc"
IMPORT "@stdlib/io/fs.arc"
IMPORT "@stdlib/io/file.arc"

# create test file
VAR file = File()
file.init(file, "test_fs_tmp.txt", "w")
file.open(file)
file.close(file)

assert_true(fs_exists("test_fs_tmp.txt"), "fs_exists on created file")

VAR del = fs_delete("test_fs_tmp.txt")
assert_eq(del, FS_OK, "fs_delete returns FS_OK")

assert_true(NOT fs_exists("test_fs_tmp.txt"), "file gone after delete")

# delete nonexistent raises error
TRY
  fs_delete("nonexistent_abc.txt")
  assert_true(0, "should not reach here")
CATCH e THEN
  assert_true(1, "fs_delete nonexistent raises error")
END

print("test_stdlib_fs.arc passed\n")
