IMPORT "@stdlib/assert.arc"
IMPORT "__time"

# perf_counter returns a float
VAR t = perf_counter()
assert_true(typeof(t) == "float", "perf_counter returns float")
assert_true(t > 0, "perf_counter > 0")

# perf_counter increases over time
VAR t1 = perf_counter()
VAR i = 0
WHILE i < 100000 THEN
  i = i + 1
END
VAR t2 = perf_counter()
assert_true(t2 > t1, "perf_counter increases")

# sleep runs without error
VAR before = perf_counter()
sleep(10)
VAR after = perf_counter()
assert_true(after > before, "sleep advances time")

print("test_time.arc passed\n")
