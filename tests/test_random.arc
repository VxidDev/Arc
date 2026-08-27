IMPORT "@stdlib/assert.arc"
IMPORT "__random"

# randint returns an int
VAR r = randint(100)
assert_true(typeof(r) == "int", "randint returns int")

# randint result is in range [0, end)
VAR all_in_range = 1
VAR i = 0
WHILE i < 200 THEN
  VAR val = randint(10)
  IF val < 0 OR val >= 10 THEN
    all_in_range = 0
  END
  i = i + 1
END
assert_eq(all_in_range, 1, "randint values in [0, end)")

# randint(1) always returns 0
assert_eq(randint(1), 0, "randint(1) == 0")

# randint with large end
VAR big = randint(1000000)
assert_true(typeof(big) == "int", "randint large end returns int")
assert_true(big >= 0 AND big < 1000000, "randint large end in range")

print("test_random.arc passed\n")
