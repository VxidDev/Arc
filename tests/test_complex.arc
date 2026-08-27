IMPORT "@stdlib/assert.arc"

# basic arithmetic
VAR a = 1
VAR b = 2
assert_eq(a + b, 3, "addition")

# loop accumulation
VAR i = 0
VAR sum = 0
WHILE i < 10 THEN
  sum = sum + i
  i = i + 1
END
assert_eq(sum, 45, "loop sum 0..9")

# recursion
FN fact(n) THEN
  IF n <= 1 THEN
    RETURN 1
  END
  RETURN n * fact(n - 1)
END

assert_eq(fact(5), 120, "factorial(5)")
assert_eq(fact(1), 1, "factorial(1)")
assert_eq(fact(10), 3628800, "factorial(10)")

# nested function calls stress frame management
FN add(a, b) THEN
  RETURN a + b
END

FN mul(a, b) THEN
  RETURN a * b
END

assert_eq(add(mul(2, 3), mul(4, 5)), 26, "nested add(mul, mul)")

print("test_complex.arc passed\n")
