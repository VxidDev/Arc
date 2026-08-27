IMPORT "@stdlib/assert.arc"

CLASS Person
  VAR name = "placeholder"

  FN say(self, str) THEN
    RETURN self.name + " said " + str
  END

  FN greet(name) THEN
    RETURN "Hello, " + name + "!"
  END

  VAR functions = [say, greet]
END

VAR test = Person()
assert_true(test.name == "placeholder", "default field value")

test.name = "Bob"
assert_true(test.name == "Bob", "field assignment")

assert_true(test.say(test, "Hello!") == "Bob said Hello!", "method call via field")
assert_true(test.greet("World") == "Hello, World!", "method call")

# function stored in list
assert_true(test.functions[0](test, "Hi") == "Bob said Hi", "functions[0] call")
assert_true(test.functions[1]("Arc") == "Hello, Arc!", "functions[1] call")

# fresh instance
VAR fresh = Person()
assert_true(fresh.name == "placeholder", "fresh instance has defaults")

# instance method on different instance
fresh.name = "Alice"
assert_true(fresh.say(fresh, "Hi") == "Alice said Hi", "different instance method")

print("test_classes.arc passed\n")
