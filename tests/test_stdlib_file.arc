IMPORT "../stdlib/io/file.arc"
IMPORT "__sys"

fn analyze_file_object(file) then 
  if typeof(file) != "instance" then 
    RuntimeError("Expected object of type 'file', received:", typeof(file))
  end 

  print("file.is_open:", if file.is_open then "true" else "false" end)
  print("file._file_obj:", file._file_obj)
  print("file.filename: \"" + file.filename + "\"")
  print("file.mode: \"" + file.mode + "\"")

  print("file.is_initialized(file):", if file.is_initialized(file) then "true" else "false" end)
end 

var f = File()

print("fresh instance:\n ")
analyze_file_object(f)

# all of those should raise error, since file is not initialized

try 
  if f.open(f) then  
    print("f.open(f)")
    exit(1)
  end 
catch e THEN 
end 

try 
  if f.close(f) then
    print("f.close(f)")
    exit(1)
  end
catch e then 
end 

try 
  if f.read(f) then 
    print("f.read(f)")
    exit(1)
  end 
catch e then 
end 

try 
  if f.write(f) then 
    print("f.write(f)")
    exit(1)
  end 
catch e then 
end

try 
  if f.read_char(f) then 
    print("f.read_char(f)")
    exit(1)
  end 
catch e then 
end 

print("\nFile() functions correctly check whether file is initialized")

f.init(f, "test.txt", "w")

print("\ninitialized instance:\n ")
analyze_file_object(f)

print("\n--- testing lifecycle correctness ---")

# open before init should fail
try 
  f2 = File()
  f2.open(f2)
  print("ERROR: open before init succeeded")
  exit(1)
catch e then 
end 

# open after init
f.open(f)

print("\nafter open:")
analyze_file_object(f)

# double open should fail
try 
  f.open(f)
  print("ERROR: double open succeeded")
  exit(1)
catch e then 
end 

# write should work
try 
  f.write(f, "hello world")
catch e then 
  print("ERROR: write failed on open file")
  exit(1)
end 

# close file
f.close(f)

print("\nafter close:")
analyze_file_object(f)

# write after close should fail
try 
  f.write(f, "test")
  print("ERROR: write after close succeeded")
  exit(1)
catch e then 
end 

# read after close should fail
try 
  f.read(f)
  print("ERROR: read after close succeeded")
  exit(1)
catch e then 
end 

try 
  f.read_char(f)
  print("ERROR: read_char after close succeeded")
  exit(1)
catch e then 
end 

# double close should fail
try 
  f.close(f)
  print("ERROR: double close succeeded")
  exit(1)
catch e then 
end

print("lifecycle correctness test passed.")

print("\n--- testing seek/tell correctness ---")

var f3 = File()
f3.init(f3, "test.txt", "w")
f3.open(f3)

f3.write(f3, "abcdef")

f3.seek(f3, 0, FILE_SEEK_END)
var end_pos = f3.tell(f3)

if end_pos <= 0 then
  print("ERROR: invalid file size")
  exit(1)
end 

f3.seek(f3, 0, FILE_SEEK_SET)

if f3.tell(f3) != 0 then
  print("ERROR: SEEK_SET failed")
  exit(1)
end 

f3.seek(f3, 2, FILE_SEEK_SET)

if f3.tell(f3) != 2 then
  print("ERROR: SEEK_SET offset failed")
  exit(1)
end 

f3.close(f3)

print("seek/tell correctness test passed.")

print("\n--- testing invalid seek mode ---")

var f4 = File()
f4.init(f4, "test.txt", "w")
f4.open(f4)

try 
  f4.seek(f4, 0, 9999)
  print("ERROR: invalid seek mode succeeded")
  exit(1)
catch e then 
end 

f4.close(f4)

print("invalid seek test passed.")

print("\n--- testing negative SEEK_SET ---")

var f5 = File()
f5.init(f5, "test.txt", "w")
f5.open(f5)

try 
  f5.seek(f5, -5, FILE_SEEK_SET)
  print("ERROR: negative SEEK_SET succeeded")
  exit(1)
catch e then 
end 

f5.close(f5)

print("negative SEEK_SET test passed.")

print("\n--- testing get_size ---")

var f6 = File()
f6.init(f6, "test.txt", "w")
f6.open(f6)

f6.write(f6, "hello world")

var size = f6.get_size(f6)

if size != 11 then
  print("ERROR: size mismatch")
  exit(1)
end 

f6.close(f6)

print("get_size test passed.")

print("\n--- testing seek state persistence ---")

var f7 = File()
f7.init(f7, "test.txt", "w")
f7.open(f7)

f7.seek(f7, 3, FILE_SEEK_SET)
f7.write(f7, "X")

if f7.tell(f7) <= 0 then
  print("ERROR: position not updated correctly")
  exit(1)
end 

f7.close(f7)

print("seek state persistence test passed.")

print("\n--- testing read correctness ---")

var f8 = File()
f8.init(f8, "test.txt", "w+")
f8.open(f8)

f8.write(f8, "abc")

f8.seek(f8, 0, FILE_SEEK_SET)

var a = f8.read_char(f8)
var b = f8.read_char(f8)
var c = f8.read_char(f8)

if a != "a" or b != "b" or c != "c" then
  print("ERROR: read_char mismatch")
  exit(1)
end 

f8.close(f8)

print("read correctness test passed.")

print("\n--- testing EOF behavior ---")

var f9 = File()
f9.init(f9, "test.txt", "w")
f9.open(f9)

f9.write(f9, "a")
f9.seek(f9, 0, FILE_SEEK_END)

var eof = f9.read_char(f9)

if eof != -1 then
  print("ERROR: EOF not handled correctly")
  exit(1)
end 

f9.close(f9)

print("EOF behavior test passed.")

print("\n--- testing invalid mode ---")

var f10 = File()
f10.init(f10, "test.txt", "invalid")

try 
  f10.open(f10)
  print("ERROR: invalid mode accepted")
  exit(1)
catch e then 
end 

print("invalid mode test passed.")

print("\n--- testing reopen persistence ---")

var f11 = File()
f11.init(f11, "test.txt", "r+")
f11.open(f11)
f11.write(f11, "abc")
f11.close(f11)

f11.open(f11)
f11.seek(f11, 0, FILE_SEEK_SET)

var r = f11.read_char(f11)

if r != "a" then
  print("ERROR: reopen persistence failed")
  exit(1)
end 

f11.close(f11)

print("reopen persistence test passed.")

print("\nFile lifecycle tests passed")
