IMPORT "@stdlib/assert.arc"

# nested try/catch
VAR inner_caught = 0
VAR outer_ran = 0

TRY
  TRY
    RuntimeError("inner")
  CATCH e THEN
    inner_caught = 1
  END
  outer_ran = 1
CATCH e THEN
  outer_ran = -1
END

assert_eq(inner_caught, 1, "nested try: inner caught")
assert_eq(outer_ran, 1, "nested try: outer continues")

# try/catch inside loop
VAR errors = 0
VAR vals = [1, 2, 0, 4, 0]
VAR results = []

FOR v IN vals THEN
  TRY
    IF v == 0 THEN
      RuntimeError("zero")
    END
    append_list(results, 10 / v)
  CATCH e THEN
    errors = errors + 1
  END
END

assert_eq(errors, 2, "try/catch in loop: 2 errors caught")
assert_eq(len_of(results), 3, "try/catch in loop: 3 results")

# try/catch in function
FN safe_operation(x) THEN
  TRY
    IF x < 0 THEN
      RuntimeError("negative")
    END
    RETURN x * 2
  CATCH e THEN
    RETURN -1
  END
END

assert_eq(safe_operation(5), 10, "safe_operation: positive")
assert_eq(safe_operation(-1), -1, "safe_operation: negative caught")

# try/catch with return in catch
FN do_or_die() THEN
  TRY
    RuntimeError("fail")
  CATCH e THEN
    RETURN "recovered"
  END
  RETURN "never"
END

assert_true(do_or_die() == "recovered", "return in catch block")

# error message propagation
VAR msg = ""
TRY
  RuntimeError("test error message")
CATCH e THEN
  msg = e
END
assert_true(msg == "test error message", "error message preserved")

# multiple catch attempts
VAR attempt = 0
TRY
  attempt = attempt + 1
  RuntimeError("first")
CATCH e THEN
  attempt = attempt + 1
END
assert_eq(attempt, 2, "catch increments attempt counter")

print("test_error_handling.arc passed\n")
