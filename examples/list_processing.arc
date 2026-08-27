IMPORT "@stdlib/assert.arc"

# creating lists
VAR fruits = ["apple", "banana", "cherry"]
print("Fruits: " + to_string(fruits))

# accessing elements
print("First: " + fruits[0])
print("Last: " + fruits[2])

# list length
print("Count: " + to_string(len_of(fruits)))

# appending
append_list(fruits, "date")
print("After append: " + to_string(fruits))

# iterating
VAR result = ""
VAR i = 0
WHILE i < len_of(fruits) THEN
  result = result + fruits[i]
  IF i < len_of(fruits) - 1 THEN
    result = result + ", "
  END
  i = i + 1
END
print("Joined: " + result)

# using range to generate numbers
VAR squares = []
VAR j = 1
WHILE j <= 10 THEN
  append_list(squares, j * j)
  j = j + 1
END
print("Squares: " + to_string(squares))

# finding max manually
VAR numbers = [3, 7, 2, 9, 4, 1, 8]
VAR max_val = numbers[0]
VAR k = 1
WHILE k < len_of(numbers) THEN
  IF numbers[k] > max_val THEN
    max_val = numbers[k]
  END
  k = k + 1
END
print("Max: " + to_string(max_val))

# reversing a list
VAR original = [1, 2, 3, 4, 5]
VAR reversed_list = []
VAR idx = len_of(original) - 1
WHILE idx >= 0 THEN
  append_list(reversed_list, original[idx])
  idx = idx - 1
END
print("Reversed: " + to_string(reversed_list))

# building a list from strings
VAR words = ["Arc", "is", "a", "language"]
VAR sentence = ""
VAR w = 0
WHILE w < len_of(words) THEN
  IF w > 0 THEN
    sentence = sentence + " "
  END
  sentence = sentence + words[w]
  w = w + 1
END
print(sentence)
