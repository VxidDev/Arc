# string concatenation
VAR greeting = "Hello" + ", " + "World!"
print(greeting)

# string multiplication
VAR line = "-" * 30
print(line)

# string length
print("Length: " + to_string(len_of("hello")))

# building strings with a buffer
VAR buf = string_buffer()
VAR i = 65
WHILE i <= 90 THEN
  append_char(buf, char_to_string(i))
  i = i + 1
END
VAR alphabet = string_finish(buf)
print("Alphabet: " + alphabet)

# splitting strings
VAR csv = "red,green,blue"
VAR colors = split_string(csv, ",")
VAR c = 0
WHILE c < len_of(colors) THEN
  print("Color " + to_string(c) + ": " + colors[c])
  c = c + 1
END

# character codes
VAR ch = char_at("Arc", 0)
print("'A' code: " + to_string(ch))

# converting code back to string
VAR letter = char_to_string(65)
print("Code 65 = " + letter)

# string comparison
IF "apple" < "banana" THEN
  print("apple comes before banana")
END

# building a string from numbers
VAR nums = ""
VAR n = 1
WHILE n <= 5 THEN
  nums = nums + to_string(n)
  IF n < 5 THEN
    nums = nums + ", "
  END
  n = n + 1
END
print("Numbers: " + nums)

# repeating a pattern
VAR pattern = ("ab" + " ") * 5
print("Pattern: " + pattern)
