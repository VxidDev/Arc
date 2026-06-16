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
  # pass  
end 

try 
  if f.close(f) then
    print("f.close(f)")
    exit(1)
  end
catch e then 
  # pass 
end 

try 
  if f.read(f) then 
    print("f.read(f)")
    exit(1)
  end 
catch e then 
  # pass 
end 

try 
  if f.write(f) then 
    print("f.write(f)")
    exit(1)
  end 
catch e then 
  # pass 
end 

print("\nFile() functions correctly check whether file is initialized")

f.init(f, "test.txt", "r")

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
  # expected
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
  # expected
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
  # expected
end 

# read after close should fail
try 
  f.read(f)
  print("ERROR: read after close succeeded")
  exit(1)
catch e then 
  # expected
end 

# double close should fail
try 
  f.close(f)
  print("ERROR: double close succeeded")
  exit(1)
catch e then 
  # expected
end 

print("\nFile lifecycle tests passed")
