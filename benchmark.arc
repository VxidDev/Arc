import "__time"
import "stdlib/json/json.arc"

fn bench(name, startTime, result) then
  var endTime = perf_counter()
  var elapsed = (endTime - startTime) * 1000
  print(name + ": " + to_string(elapsed) + "ms (result: " + to_string(result) + ")")
end

# 1. fibonacci recursive
fn fib(n) then
  if n <= 1 then
    return n
  end
  return fib(n - 1) + fib(n - 2)
end

var t = perf_counter()
var r = fib(30)
bench("fibonacci(30) recursive", t, r)

# 2. fibonacci iterative
fn fibIter(n) then
  var a = 0
  var b = 1
  var i = 0
  while i < n then
    var tmp = b
    b = a + b
    a = tmp
    i = i + 1
  end
  return a
end

var t = perf_counter()
var r = fibIter(1000)
bench("fibonacci(1000) iterative", t, r)

# 3. sum range
fn sumRange(n) then
  var s = 0
  var i = 0
  while i < n then
    s = s + i
    i = i + 1
  end
  return s
end

var t = perf_counter()
var r = sumRange(1000000)
bench("sum 1..1000000", t, r)

# 4. string concat
fn strConcat(n) then
  var s = ""
  var i = 0

  while i < n then
    s = s + "x"
    i = i + 1
  end

  return len_of(s)
end

var t = perf_counter()
var r = strConcat(10000)
bench("string concat x10000", t, r)

# 6. build list
fn build_list(n) then
  lst = []
  i = 0

  while i < n then
    lst = append_list(lst, i)
    i = i + 1
  end 

  return len_of(lst)
end

var t = perf_counter()
var r = build_list(1000)

bench("list build x1000", t, r)

# 5. nested loops
fn nestedLoops(n) then
  var s = 0
  var i = 0
  while i < n then
    var j = 0
    while j < n then
      s = s + 1
      j = j + 1
    end
    i = i + 1
  end
  return s
end

var t = perf_counter()
var r = nestedLoops(1000)
bench("nested loops 1000x1000", t, r)

# 6. recursive countdown
fn countdown(n) then
  if n <= 0 then
    return 0
  end
  return countdown(n - 1)
end

var t = perf_counter()
var r = countdown(5000)
bench("recursive countdown 5000", t, r)

# 7. variable churn
fn varChurn(n) then
  var x = 0
  var i = 0
  while i < n then
    x = x + i
    x = x * 1
    x = x - 0
    i = i + 1
  end
  return x
end

var t = perf_counter()
var r = varChurn(100000)
bench("variable churn x100000", t, r)

var json_string = null

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
  bench("nested json (3.7 mb)", t, s)# "skipped printing result")
end
