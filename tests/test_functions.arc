IMPORT "@stdlib/assert.arc"

# function with return value
FN add(a, b) THEN
  RETURN a + b
END

assert_eq(add(2, 3), 5, "simple function call")
assert_eq(add(0, 0), 0, "function returning zero")
assert_eq(add(-1, 1), 0, "function with negatives")

# function with multiple statements
FN compute(x) THEN
  VAR y = x * 2
  VAR z = y + 1
  RETURN z
END

assert_eq(compute(5), 11, "multi-statement function")
assert_eq(compute(0), 1, "multi-statement edge case")

# function calling another function
FN double(x) THEN
  RETURN x * 2
END

FN quadruple(x) THEN
  RETURN double(double(x))
END

assert_eq(quadruple(3), 12, "function calling function")
assert_eq(quadruple(0), 0, "nested calls edge case")

# recursion
FN factorial(n) THEN
  IF n <= 1 THEN
    RETURN 1
  END
  RETURN n * factorial(n - 1)
END

assert_eq(factorial(1), 1, "factorial(1)")
assert_eq(factorial(5), 120, "factorial(5)")
assert_eq(factorial(10), 3628800, "factorial(10)")

# recursive fibonacci
FN fib(n) THEN
  IF n <= 1 THEN
    RETURN n
  END
  RETURN fib(n - 1) + fib(n - 2)
END

assert_eq(fib(0), 0, "fib(0)")
assert_eq(fib(1), 1, "fib(1)")
assert_eq(fib(10), 55, "fib(10)")

# function stored in variable
VAR fn_var = add
assert_true(fn_var(3, 4) == 7, "function stored in variable")

# function as argument (passed by reference)
FN apply(f, x, y) THEN
  RETURN f(x, y)
END

assert_true(apply(add, 10, 20) == 30, "function as argument")

# function with no explicit return returns null
FN noop() THEN
  VAR x = 1
END

# functions return last expression value
assert_eq(noop(), 1, "function returns last expression value")

# nested function definition
FN outer() THEN
  FN inner() THEN
    RETURN 42
  END
  RETURN inner()
END

assert_eq(outer(), 42, "nested function definition")

# multiple returns via if/else
FN abs_val(x) THEN
  IF x < 0 THEN
    RETURN -x
  END
  RETURN x
END

assert_eq(abs_val(-5), 5, "abs negative")
assert_eq(abs_val(5), 5, "abs positive")
assert_eq(abs_val(0), 0, "abs zero")

print("test_functions.arc passed\n")
