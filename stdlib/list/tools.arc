# argument 'pairs' must be a list of 
# list objects with 2 objects, key and value.
# for example:
# [["key", "value"], ["abc", 4], ["test", 10]]
fn get_item(pairs, key) then
  for pair in pairs then
    if pair[0] == key then
      return pair[1]
    end
  end
end
