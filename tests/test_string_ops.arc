IMPORT "__sys"

# Test string concatenation (+)
var a = "Hello"
var b = " World"
var c = a + b
print("Concatenation: ", c == "Hello World")

if c != "Hello World" then exit(1) end

print("Result: ", c)

# Test string multiplication (*)
var d = "Arc"
var e = d * 3
print("Multiplication: ", e == "ArcArcArc")

if e != "ArcArcArc" then exit(1) end 

print("Result: ", e)

# Test equality and mutability interaction
var s1 = "test"
var s2 = "test"

print("Initial equality: ", s1 == s2)

if s1 != s2 then exit(1) end 

s1[0] = "T"
print("Modified s1: ", s1)
print("s2 remains: ", s2)
print("Equality after mutation: ", s1 == s2)

if s1 == s2 then exit(1) end 

# Test edge cases
var empty = ""

print("Empty + String: ", (empty + "foo") == "foo")

if (empty + "foo") != "foo" then exit(1) end 

print("String * 0: ", (d * 0) == "")

if (d * 0) != "" then exit(1) end 

print("String * 1: ", (d * 1) == "Arc")

if (d * 1) != "Arc" then exit(1) end

# Test list of strings
var list = ["a", "b", "c"]
var joined = list[0] + list[1] + list[2]
print("Joined list elements: ", joined == "abc")

if joined != "abc" then exit(1) end

print("\ntest_string_ops.arc passed")
