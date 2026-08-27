IMPORT "@stdlib/assert.arc"
IMPORT "@stdlib/math.arc"

# nested while loops
VAR sum = 0
VAR i = 0
WHILE i < 10 THEN
  VAR j = 0
  WHILE j < 10 THEN
    sum = sum + 1
    j = j + 1
  END
  i = i + 1
END
assert_eq(sum, 100, "nested while loops 10x10")

# while with complex condition
VAR x = 0
VAR y = 10
WHILE x < y AND y > 0 THEN
  x = x + 1
  y = y - 1
END
assert_eq(x, 5, "while with compound condition")
assert_eq(y, 5, "while compound condition end state")

# while with early break
VAR count = 0
VAR n = 0
WHILE n < 1000 THEN
  n = n + 1
  IF n == 50 THEN
    BREAK
  END
  count = count + 1
END
assert_eq(count, 49, "while break skips remaining iterations")
assert_eq(n, 50, "while break stops at correct value")

# while with continue
VAR evens = 0
VAR m = 0
WHILE m < 20 THEN
  m = m + 1
  IF fmod(m, 2) != 0 THEN
    CONTINUE
  END
  evens = evens + 1
END
assert_eq(evens, 10, "while continue counts evens 2..20")

# for loop with break at start
VAR first = 0
FOR v IN [10, 20, 30, 40, 50] THEN
  first = v
  BREAK
END
assert_eq(first, 10, "for break on first element")

# for loop with continue skipping middle
VAR skipped = []
FOR v IN [1, 2, 3, 4, 5] THEN
  IF v == 3 THEN
    CONTINUE
  END
  append_list(skipped, v)
END
assert_eq(len_of(skipped), 4, "for continue skips one element")
assert_eq(skipped[0], 1, "for continue: first element")
assert_eq(skipped[2], 4, "for continue: skips 3")

# nested for loops
VAR pairs = []
VAR a = 0
WHILE a < 3 THEN
  VAR b = 0
  WHILE b < 3 THEN
    append_list(pairs, [a, b])
    b = b + 1
  END
  a = a + 1
END
assert_eq(len_of(pairs), 9, "nested for: 3x3 pairs")
assert_true(pairs[0][0] == 0 AND pairs[0][1] == 0, "nested for: first pair")
assert_true(pairs[8][0] == 2 AND pairs[8][1] == 2, "nested for: last pair")

print("test_loops.arc passed\n")
