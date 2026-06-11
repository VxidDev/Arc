# Test string concatenation (+)
var a = "Hello"
var b = " World"
var c = a + b
print("Concatenation: ", c == "Hello World")
print("Result: ", c)

# Test string multiplication (*)
var d = "Arc"
var e = d * 3
print("Multiplication: ", e == "ArcArcArc")
print("Result: ", e)

# Test equality and mutability interaction
var s1 = "test"
var s2 = "test"
print("Initial equality: ", s1 == s2)

s1[0] = "T"
print("Modified s1: ", s1)
print("s2 remains: ", s2)
print("Equality after mutation: ", s1 == s2)

# Test edge cases
var empty = ""
print("Empty + String: ", (empty + "foo") == "foo")
print("String * 0: ", (d * 0) == "")
print("String * 1: ", (d * 1) == "Arc")

# Test list of strings
var list = ["a", "b", "c"]
var joined = list[0] + list[1] + list[2]
print("Joined list elements: ", joined == "abc")
