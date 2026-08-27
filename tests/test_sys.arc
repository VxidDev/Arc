IMPORT "@stdlib/assert.arc"
IMPORT "__sys"

# get_os returns a string
VAR os = get_os()
assert_true(typeof(os) == "string", "get_os returns string")
assert_true(len_of(os) > 0, "get_os non-empty")

# getenv returns a string for existing key
VAR home = getenv("HOME")
assert_true(typeof(home) == "string", "getenv returns string")

# getenv returns empty string for missing key
VAR missing = getenv("ARC_TEST_MISSING_VAR_12345")
assert_true(typeof(missing) == "string", "getenv missing key returns string")
assert_eq(len_of(missing), 0, "getenv missing key returns empty")

# system returns an int
VAR rc = system("true")
assert_eq(rc, 0, "system('true') returns 0")

VAR rc2 = system("false")
assert_true(rc2 != 0, "system('false') returns non-zero")

print("test_sys.arc passed\n")
