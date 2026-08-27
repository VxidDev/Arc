# basic try/catch
TRY
  RuntimeError("something went wrong")
CATCH e THEN
  print("Caught: " + e)
END

# safe division
FN safe_divide(a, b) THEN
  TRY
    IF b == 0 THEN
      RuntimeError("division by zero")
    END
    RETURN a / b
  CATCH e THEN
    print("Error: " + e)
    RETURN null
  END
END

VAR result = safe_divide(10, 2)
print("10 / 2 = " + to_string(result))

result = safe_divide(10, 0)
IF result == null THEN
  print("10 / 0 = null (error handled)")
END

# safe file reading
FN read_file_safe(path) THEN
  IMPORT "@stdlib/io/file.arc"
  TRY
    VAR f = File()
    f.init(f, path, "r")
    f.open(f)
    VAR content = f.read(f)
    f.close(f)
    RETURN content
  CATCH e THEN
    print("Could not read " + path + ": " + e)
    RETURN null
  END
END

VAR data = read_file_safe("nonexistent.txt")
IF data == null THEN
  print("File not found, handling gracefully")
END

# error propagation pattern
FN divide_all(numbers, divisor) THEN
  VAR results = []
  VAR i = 0
  WHILE i < len_of(numbers) THEN
    TRY
      IF divisor == 0 THEN
        RuntimeError("cannot divide by zero")
      END
      append_list(results, numbers[i] / divisor)
    CATCH e THEN
      print("Skipping " + to_string(numbers[i]) + ": " + e)
    END
    i = i + 1
  END
  RETURN results
END

VAR values = [10, 20, 30, 40, 50]
print("Divided by 2: " + to_string(divide_all(values, 2)))
print("Divided by 0: " + to_string(divide_all(values, 0)))
