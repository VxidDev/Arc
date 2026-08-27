IMPORT "@stdlib/assert.arc"

# --- AND operator ---

assert_true(1 AND 1, "true AND true")
assert_true(NOT (1 AND 0), "true AND false is false")
assert_true(NOT (0 AND 1), "false AND true is false")
assert_true(NOT (0 AND 0), "false AND false is false")

# --- OR operator ---

assert_true(1 OR 1, "true OR true")
assert_true(1 OR 0, "true OR false")
assert_true(0 OR 1, "false OR true")
assert_true(NOT (0 OR 0), "false OR false is false")

# --- NOT operator ---

assert_true(NOT 0, "NOT false")
assert_true(NOT NOT 1, "NOT NOT true")

# --- Combined logic ---

assert_true(1 AND 1 OR 0, "AND before OR")
assert_true((1 OR 0) AND 1, "OR with parens")
assert_true(0 OR 0 OR 1, "OR chain")
assert_true(1 AND 1 AND 1 AND 1, "AND chain")

# --- With comparisons ---

VAR x = 5
assert_true(x > 3 AND x < 10, "range check")
assert_true(x == 5 OR x == 6, "alternatives")
assert_true(NOT (x == 3), "NOT with comparison")

# --- Edge cases ---

assert_true(0 AND 0 OR 1, "precedence: AND before OR")
assert_true(0 OR 1 AND 1, "precedence: AND before OR 2")

print("test_logical.arc passed\n")
