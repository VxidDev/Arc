IMPORT "@stdlib/assert.arc"

# return inside function: no warning, works
FN get_value() THEN
  RETURN 42
END
assert_eq(get_value(), 42, "return inside function")

# break/continue inside loop: no warnings, works
VAR sum = 0
VAR i = 0
WHILE i < 10 THEN
  i = i + 1
  IF i == 3 THEN
    CONTINUE
  END
  IF i == 7 THEN
    BREAK
  END
  sum = sum + i
END
assert_eq(sum, 18, "break/continue inside loop")

# break in function loop: no warning
FN count_to(n) THEN
  VAR count = 0
  VAR j = 0
  WHILE j < n THEN
    j = j + 1
    IF j == 5 THEN
      BREAK
    END
    count = count + 1
  END
  RETURN count
END
assert_eq(count_to(100), 4, "break in function loop")

# for loop with break: no warning
VAR total = 0
FOR i IN [1, 2, 3, 4, 5] THEN
  IF i == 4 THEN
    BREAK
  END
  total = total + i
END
assert_eq(total, 6, "for loop break")

# nested loops with break/continue: no warnings
VAR result = 0
VAR x = 0
WHILE x < 5 THEN
  x = x + 1
  VAR y = 0
  WHILE y < 5 THEN
    y = y + 1
    IF y == 2 THEN
      CONTINUE
    END
    IF x == 3 THEN
      BREAK
    END
    result = result + 1
  END
END
assert_eq(result, 16, "nested loop break/continue")

# return inside nested function: no warning
FN outer() THEN
  FN inner() THEN
    RETURN 99
  END
  RETURN inner()
END
assert_eq(outer(), 99, "nested return")

print("test_warnings.arc passed\n")
