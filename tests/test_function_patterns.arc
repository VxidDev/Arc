IMPORT "@stdlib/assert.arc"

# multiple return values via early return
FN classify(n) THEN
  IF n < 0 THEN
    RETURN "negative"
  END
  IF n == 0 THEN
    RETURN "zero"
  END
  RETURN "positive"
END

assert_true(classify(-5) == "negative", "classify negative")
assert_true(classify(0) == "zero", "classify zero")
assert_true(classify(7) == "positive", "classify positive")

# function with many parameters
FN add5(a, b, c, d, e) THEN
  RETURN a + b + c + d + e
END

assert_eq(add5(1, 2, 3, 4, 5), 15, "add5(1..5)")
assert_eq(add5(0, 0, 0, 0, 0), 0, "add5 zeros")
assert_eq(add5(-1, 1, -1, 1, 0), 0, "add5 mixed")

# function calling function with result
FN square(x) THEN
  RETURN x * x
END

FN sum_of_squares(a, b) THEN
  RETURN square(a) + square(b)
END

assert_eq(sum_of_squares(3, 4), 25, "sum_of_squares(3,4)")
assert_eq(sum_of_squares(0, 0), 0, "sum_of_squares(0,0)")

# mutual recursion
FN is_even(n) THEN
  IF n == 0 THEN
    RETURN 1
  END
  RETURN is_odd(n - 1)
END

FN is_odd(n) THEN
  IF n == 0 THEN
    RETURN 0
  END
  RETURN is_even(n - 1)
END

assert_eq(is_even(4), 1, "is_even(4)")
assert_eq(is_odd(4), 0, "is_odd(4)")
assert_eq(is_even(7), 0, "is_even(7)")
assert_eq(is_odd(7), 1, "is_odd(7)")

# function stored in list and called
FN double(x) THEN
  RETURN x * 2
END

FN triple(x) THEN
  RETURN x * 3
END

VAR ops = [double, triple]
assert_eq(ops[0](5), 10, "list[0](5) = double")
assert_eq(ops[1](5), 15, "list[1](5) = triple")

# function with default-like pattern
FN greet(name) THEN
  IF name == "" THEN
    RETURN "Hello, stranger!"
  END
  RETURN "Hello, " + name + "!"
END

assert_true(greet("Arc") == "Hello, Arc!", "greet named")
assert_true(greet("") == "Hello, stranger!", "greet anonymous")

# recursive sum
FN rsum(n) THEN
  IF n <= 0 THEN
    RETURN 0
  END
  RETURN n + rsum(n - 1)
END

assert_eq(rsum(0), 0, "rsum(0)")
assert_eq(rsum(1), 1, "rsum(1)")
assert_eq(rsum(10), 55, "rsum(10)")

# function returning function result
FN apply_twice(f, x) THEN
  RETURN f(f(x))
END

assert_eq(apply_twice(double, 3), 12, "apply_twice double(3) = 12")
assert_eq(apply_twice(square, 2), 16, "apply_twice square(2) = 16")

print("test_function_patterns.arc passed\n")
