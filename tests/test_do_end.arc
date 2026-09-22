IMPORT "@stdlib/assert.arc"
IMPORT "__sys"

VAR r = DO
  VAR a = 1
  VAR b = 2
  a + b
END

assert_eq(r, 3, "block evaluates to last value")

VAR x = 10

DO
  VAR x = 20
  assert_eq(x, 20, "inner x shadows outer")
END

assert_eq(x, 10, "outer x unchanged after block")

VAR counter = 0

DO
  counter = counter + 1
  counter = counter + 1
END

assert_eq(counter, 2, "writes reach outer variable")

VAR single = DO 7 * 6 END
assert_eq(single, 42, "single-expression block")

VAR nested = DO
  VAR i = 1
  DO
    VAR j = 10
    i + j
  END + 1
END

assert_eq(nested, 12, "nested blocks")

FN sum_upto(n) THEN
  VAR total = 0
  VAR i = 1
  WHILE i <= n THEN
    DO
      VAR step = i * 2
      total = total + step
    END
    i = i + 1
  END
  RETURN total
END

assert_eq(sum_upto(3), 12, "block in loop inside function")
