IMPORT "math.arc"
IMPORT "assert.arc"

VAR caught = 0

TRY
  RuntimeError("basic fail")
CATCH e THEN
  caught = 1
  print("Caught:", e)
END

assert_eq(caught, 1, "basic try/catch should trigger catch")

VAR no_catch = 1

TRY
  VAR x = 10
  VAR y = 20
  VAR z = x + y
CATCH e THEN
  no_catch = 0
END

assert_eq(no_catch, 1, "catch should not run when no error")

VAR msg_ok = 0

TRY
  RuntimeError("hello world")
CATCH e THEN
  IF e == "hello world" THEN
    msg_ok = 1
  END
END

assert_eq(msg_ok, 1, "error message should propagate")

VAR nested_ok = 0

TRY
  TRY
    RuntimeError("inner error")
  CATCH e THEN
    # swallow inner error
    VAR tmp = 1
  END

  # outer continues normally
  nested_ok = 1
CATCH e THEN
  nested_ok = 0
END

assert_eq(nested_ok, 1, "nested try/catch should not break outer flow")

FN risky() THEN
  TRY
    RuntimeError("function error")
  CATCH e THEN
    RETURN 42
  END

  RETURN 0
END

assert_eq(risky(), 42, "try/catch inside function should return recovery value")

VAR final_check = 0

TRY
  VAR a = 1
  VAR b = 0
  # force error
  RuntimeError("final test")
  final_check = 999 # should NOT execute
CATCH e THEN
  final_check = 7
END

assert_eq(final_check, 7, "execution must jump to catch immediately")

print("TRY/CATCH tests completed\n")
print("test_trycatch.arc passed\n")

