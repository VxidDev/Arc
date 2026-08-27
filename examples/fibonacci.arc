# recursive fibonacci
FN fib(n) THEN
  IF n <= 1 THEN
    RETURN n
  END
  RETURN fib(n - 1) + fib(n - 2)
END

# iterative fibonacci (faster)
FN fib_iter(n) THEN
  VAR a = 0
  VAR b = 1
  VAR i = 0
  WHILE i < n THEN
    VAR tmp = b
    b = a + b
    a = tmp
    i = i + 1
  END
  RETURN a
END

# print first 15 fibonacci numbers
VAR i = 0
WHILE i < 15 THEN
  print("fib(" + to_string(i) + ") = " + to_string(fib(i)))
  i = i + 1
END

# compare recursive vs iterative
IMPORT "__time"

VAR t1 = perf_counter()
VAR r1 = fib(30)
VAR t2 = perf_counter()
print("recursive fib(30): " + to_string(r1) + " in " + to_string((t2 - t1) * 1000) + "ms")

t1 = perf_counter()
VAR r2 = fib_iter(30)
t2 = perf_counter()
print("iterative fib(30): " + to_string(r2) + " in " + to_string((t2 - t1) * 1000) + "ms")
