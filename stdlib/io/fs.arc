import "__sys"

var F_OK = 0

var FS_OK = 0
var FS_NOTFOUND = 1 
var FS_PERM = 2 
var FS_UNKNOWN = 3

fn fs_exists(path) then
  if access(path, F_OK) == 0 then 
    return true 
  end 

  return false 
end

fn fs_delete(path) then 
  var res = unlink(path)

  if res == FS_OK then 
    return FS_OK 
  end 

  if res == FS_NOTFOUND then
    return RuntimeError("File not found.")
  end

  if res == FS_PERM then 
    return RuntimeError("Insufficient permissions.")
  end 

  return RuntimeError("File I/O error.")
end 
