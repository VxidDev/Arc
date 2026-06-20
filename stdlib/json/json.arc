import "../properties.arc"

class JSONObject 
  var key = ""
  var value = 0

  fn to_list(self) then 
    return [self.key, self.value]
  end 
end 

class JSONMap
  var length = 0
  var kv_pairs = []
end
