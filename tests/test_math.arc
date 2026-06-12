IMPORT "math.arc"
IMPORT "assert.arc"

assert_eq(PI, 3.141592653589793, "PI")
assert_eq(E, 2.718281828459045, "E")
assert_eq(TAU, 6.283185307179586, "TAU")

assert_eq(exp(1), E, "exp(1)")
assert_eq(logn(E), 1, "ln(e)")
assert_eq(logn(1), 0, "ln(1)")
assert_eq(log(10, 100), 2, "log base 10")

assert_eq(sqrt(9), 3, "sqrt(9)")
assert_eq(abs(-5), 5, "abs(-5)")

assert_eq(sign(-10), -1, "sign(-10)")
assert_eq(sign(0), 0, "sign(0)")
assert_eq(sign(10), 1, "sign(10)")

assert_eq(floor(3.7), 3, "floor positive")
assert_eq(floor(-3.2), -4, "floor negative")
assert_eq(fmod(10, 3), 1, "fmod basic")

assert_eq(sin(0), 0, "sin(0)")
assert_eq(cos(0), 1, "cos(0)")
assert_eq(tan(0), 0, "tan(0)")

assert_eq(logn(exp(2)), 2, "ln(exp(x)) roundtrip")

VAR a = [1, 5, 2, 9, 3]
VAR b = [-10, -2, -30]

assert_eq(max(a), 9, "max positive list")
assert_eq(min(a), 1, "min positive list")
assert_eq(max(b), -2, "max negative list")
assert_eq(min(b), -30, "min negative list")

assert_eq(logn(-5), 0, "ln negative safety")
assert_eq(log(2, -10), 0, "log invalid base/x safety")
assert_eq(fmod(5, 0), 0, "fmod divide by zero")

VAR i = 0
VAR acc = 0

WHILE i < 10000 THEN
  acc = acc + sin(i)
  i = i + 1
END

assert_true(acc != 0, "stress sin loop")

print("test_math.arc passed\n")

