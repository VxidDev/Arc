VAR EPS = 0.0001

FN assert_eq(a, b, msg) THEN
  IF abs(a - b) > EPS THEN
    RuntimeError("FAIL: " + msg + " | expected " + to_string(b) + " got " + to_string(a))
  END
END

FN assert_true(cond, msg) THEN
  IF NOT cond THEN
    RuntimeError("FAIL: " + msg)
  END
END
