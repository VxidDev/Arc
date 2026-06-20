fn is_int(str) then 
  try 
    to_int(str)
    return true 
  catch e then 
    return false
  end 
end 
