IMPORT "@stdlib/assert.arc"

VAR i = 0
WHILE i < 100 THEN
  i = i + 1
END
assert_eq(i, 100, "while loop runs 100 iterations")

# false condition skips body
VAR ran = 0
WHILE 0 THEN
  ran = 1
END
assert_eq(ran, 0, "false condition skips body")

# break exits early
VAR j = 0
WHILE j < 100 THEN
  IF j == 10 THEN
    BREAK
  END
  j = j + 1
END
assert_eq(j, 10, "break at 10")

# continue skips iteration
VAR collected = 0
VAR k = 0
WHILE k < 10 THEN
  k = k + 1
  IF k == 5 THEN
    CONTINUE
  END
  collected = collected + k
END
assert_eq(collected, 50, "continue skips 5 (sum 1..10 minus 5 = 50)")

print("test_while.arc passed\n")
