CLASS Person
  VAR name = "placeholder"

  FN say(self, str) THEN
    print(self.name, "said:", str)
  END 

  FN greet(name) THEN
    print("Hello, " + name + "!")
  END 

  VAR functions = [say, greet]
END

VAR test = Person()

print("(before assignment) test.name = " + test.name)

test.name = "Bob"

print("(after assignment) test.name = " + test.name)

print("test.functions[0](test, \"Hello!\"):")
test.functions[0](test, "Hello!")

print("test.greet(\"World\")")
test.greet("World")

print("Person().functions[1](\"World\")")
Person().functions[1]("World")

print("\ntest_classes.arc passed.")
