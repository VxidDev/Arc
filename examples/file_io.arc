IMPORT "@stdlib/io/file.arc"

# write to a file
VAR f = File()
f.init(f, "example_output.txt", "w")
f.open(f)
f.write(f, "Hello from Arc!\n")
f.write(f, "This file was created by the file_io example.\n")
f.close(f)
print("Wrote to example_output.txt")

# read it back
VAR reader = File()
reader.init(reader, "example_output.txt", "r")
reader.open(reader)
VAR content = reader.read(reader)
reader.close(reader)
print("Content: " + content)

# write a list of numbers
VAR nums = File()
nums.init(nums, "numbers.txt", "w")
nums.open(nums)
VAR i = 1
WHILE i <= 10 THEN
  nums.write(nums, to_string(i) + "\n")
  i = i + 1
END
nums.close(nums)
print("Wrote numbers.txt")

# read numbers back and sum them
VAR rf = File()
rf.init(rf, "numbers.txt", "r")
rf.open(rf)
VAR total = 0
WHILE 1 THEN
  VAR ch = rf.read_char(rf)
  IF ch == -1 THEN
    BREAK
  END
  # simple digit parsing for single-digit numbers
  VAR code = char_at(to_string(ch), 0)
  IF code >= 48 AND code <= 57 THEN
    total = total + (code - 48)
  END
END
rf.close(rf)
print("Sum of digits: " + to_string(total))

# cleanup
IMPORT "@stdlib/io/fs.arc"
fs_delete("example_output.txt")
fs_delete("numbers.txt")
print("Cleaned up temp files")
