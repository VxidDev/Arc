VAR a = 1
VAR b = 2

IF a + b == 3 THEN
  print("Addition works\n")
END

VAR i = 0
VAR sum = 0

WHILE i < 10 THEN
  sum = sum + i
  i = i + 1
END

IF sum == 45 THEN
  print("Loop and accumulation work\n")
END

# Nested function calls to stress frame management
FN fact(n) THEN
  IF n <= 1 THEN
    RETURN 1
  END

  RETURN n * fact(n - 1)
END

VAR res = fact(5)

IF res == 120 THEN
  print("Recursion and frames work\n")
END

print("test_complex.arc passed\n")
