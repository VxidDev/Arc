IMPORT "@stdlib/assert.arc"

# print returns 1
VAR ret = print("hello")
assert_eq(ret, 1, "print returns 1")

# print with multiple args
VAR ret2 = print("a", "b", "c")
assert_eq(ret2, 1, "print multiple args returns 1")

# open_file / close_file
VAR f = open_file("test_io_tmp.txt", "w")
assert_true(typeof(f) == "file", "open_file returns file")

# write_file
VAR w = write_file(f, "hello world")
assert_eq(w, 1, "write_file returns 1")

# close_file
VAR c = close_file(f)
assert_eq(c, 1, "close_file returns 1")

# open for reading
VAR reader = open_file("test_io_tmp.txt", "r")
VAR content = read_file(reader)
assert_true(content == "hello world", "read_file content matches")

# stream_tell
VAR pos = stream_tell(reader)
assert_true(pos >= 0, "stream_tell returns non-negative")

# stream_read_char
stream_seek(reader, 0, 0)
VAR ch1 = stream_read_char(reader)
assert_true(ch1 == "h", "stream_read_char first char")

VAR ch2 = stream_read_char(reader)
assert_true(ch2 == "e", "stream_read_char second char")

# stream_seek back to start
stream_seek(reader, 0, 0)
VAR ch3 = stream_read_char(reader)
assert_true(ch3 == "h", "stream_seek + read_char resets")

# close
close_file(reader)

# error: open nonexistent for read
TRY
  VAR bad = open_file("nonexistent_io_test.txt", "r")
  assert_true(0, "should not reach here")
CATCH e THEN
  assert_true(1, "open_file nonexistent raises error")
END

# cleanup
IMPORT "@stdlib/io/fs.arc"
fs_delete("test_io_tmp.txt")

print("test_io_builtins.arc passed\n")
