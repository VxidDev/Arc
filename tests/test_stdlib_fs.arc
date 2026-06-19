import "../stdlib/io/fs.arc"
import "../stdlib/io/file.arc"
# import "__sys" included with fs.arc 

# create test file 
var file = File()
file.init(file, "test.txt", "w")
file.open(file)
file.close(file)

if fs_exists("test.txt") != true then 
  print("fs_exists(test.txt)")
  exit(1)
end 

if fs_delete("test.txt") != FS_OK then 
  print("fs_delete(test.txt)")
  exit(1)
end 

try 
  fs_delete("abc.txt") 
  print("fs_delete(abc.txt) # unexistent")
  exit(1)
catch e then 
end 

print("test_stdlib_fs.arc passed\n")
