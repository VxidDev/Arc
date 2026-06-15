IMPORT "../stdlib/assert.arc"
IMPORT "__sys"

# basic if true
VAR result = 0

IF 1 THEN
  result = 1
END

assert_true(result == 1, "if true executes then branch")

# basic if false
result = 0

IF 0 THEN
  result = 1
END

assert_true(result == 0, "if false skips then branch")

# if/else true branch
result = 0

IF 1 THEN
  result = 1
ELSE
  result = 2
END

assert_eq(result, 1, "if/else takes then branch when true")

# if/else false branch
result = 0

IF 0 THEN
  result = 1
ELSE
  result = 2
END

assert_eq(result, 2, "if/else takes else branch when false")

# elif first condition true
result = 0

IF 1 THEN
  result = 1
ELIF 1 THEN
  result = 2
ELSE
  result = 3
END

assert_eq(result, 1, "elif: first condition wins")

# elif second condition true
result = 0

IF 0 THEN
  result = 1
ELIF 1 THEN
  result = 2
ELSE
  result = 3
END

assert_eq(result, 2, "elif: second condition taken when first false")

# elif falls to else
result = 0

IF 0 THEN
  result = 1
ELIF 0 THEN
  result = 2
ELSE
  result = 3
END

assert_eq(result, 3, "elif: else taken when all conditions false")

# multiple elif chains
result = 0
VAR x = 3

IF x == 1 THEN
  result = 1
ELIF x == 2 THEN
  result = 2
ELIF x == 3 THEN
  result = 3
ELIF x == 4 THEN
  result = 4
ELSE
  result = 5
END

assert_eq(result, 3, "multiple elif chain picks correct branch")

# nested if inside then
result = 0

IF 1 THEN
  IF 1 THEN
    result = 1
  END
END
assert_eq(result, 1, "nested if inside then branch")

# nested if inside else
result = 0

IF 0 THEN
  result = 1
ELSE
  IF 1 THEN
    result = 2
  END
END
assert_eq(result, 2, "nested if inside else branch")

# nested if/else inside elif
result = 0

IF 0 THEN
  result = 1
ELIF 1 THEN
  IF 0 THEN
    result = 2
  ELSE
    result = 3
  END
ELSE
  result = 4
END

assert_eq(result, 3, "nested if/else inside elif branch")

# if with expression condition
result = 0

IF 2 + 2 == 4 THEN
  result = 1
END

assert_true(result == 1, "if with expression condition")

# if with negation
result = 0

IF NOT 0 THEN
  result = 1
END

assert_true(result == 1, "if NOT false is true")

# if/elif with no else, condition false, no crash
result = 99

IF 0 THEN
  result = 1
ELIF 0 THEN
  result = 2
END

assert_eq(result, 99, "if/elif with no else leaves result unchanged")

print("all IF/ELIF/ELSE tests passed")
print("test_if.arc passed\n")

