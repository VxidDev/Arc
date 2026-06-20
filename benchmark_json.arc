import "__time"
import "stdlib/json/json.arc"

fn bench(name, startTime, result) then
  var endTime = perf_counter()
  var elapsed = (endTime - startTime) * 1000
  print(name + ": " + to_string(elapsed) + "ms (result: " + to_string(result) + ")")
end

try
  var file = open_file("nested.json", "r")
  json_string = read_file(file)
  close_file(file)
catch e then 
  print("nested.json not found, skipping benchmark")
end 

if json_string then 
  var t = perf_counter()
  var s = to_json(json_string)
  bench("nested json (3.7 mb)", t, "skipped printing result")
end
