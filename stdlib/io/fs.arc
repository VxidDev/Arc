import "__sys"

var F_OK = 0

fn fs_exists(path) then
  if access(path, F_OK) == 0 then 
    return true 
  end 

  return false 
end 
