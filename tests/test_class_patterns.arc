IMPORT "@stdlib/assert.arc"

# class with static-like pattern
CLASS Counter
  VAR count = 0

  FN increment(self) THEN
    self.count = self.count + 1
    RETURN self
  END

  FN get(self) THEN
    RETURN self.count
  END

  FN reset(self) THEN
    self.count = 0
    RETURN self
  END
END

VAR c1 = Counter()
c1.increment(c1)
c1.increment(c1)
c1.increment(c1)
assert_eq(c1.get(c1), 3, "counter 3 increments")

VAR c2 = Counter()
c2.increment(c2)
assert_eq(c1.get(c1), 3, "c1 unaffected by c2")
assert_eq(c2.get(c2), 1, "c2 independent")

c1.reset(c1)
assert_eq(c1.get(c1), 0, "counter reset")

# class with computed property
CLASS Rectangle
  VAR width = 0
  VAR height = 0

  FN init(self, w, h) THEN
    self.width = w
    self.height = h
    RETURN self
  END

  FN area(self) THEN
    RETURN self.width * self.height
  END

  FN perimeter(self) THEN
    RETURN 2 * (self.width + self.height)
  END

  FN is_square(self) THEN
    RETURN self.width == self.height
  END
END

VAR r1 = Rectangle()
r1.init(r1, 5, 3)
assert_eq(r1.area(r1), 15, "rectangle area")
assert_eq(r1.perimeter(r1), 16, "rectangle perimeter")
assert_true(r1.is_square(r1) == 0, "5x3 not square")

VAR r2 = Rectangle()
r2.init(r2, 4, 4)
assert_true(r2.is_square(r2) == 1, "4x4 is square")

# class with list management
CLASS Stack
  VAR items = []

  FN push(self, item) THEN
    append_list(self.items, item)
    RETURN self
  END

  FN pop(self) THEN
    VAR last = self.items[len_of(self.items) - 1]
    pop_list(self.items)
    RETURN last
  END

  FN peek(self) THEN
    RETURN self.items[len_of(self.items) - 1]
  END

  FN size(self) THEN
    RETURN len_of(self.items)
  END

  FN is_empty(self) THEN
    RETURN len_of(self.items) == 0
  END
END

VAR s = Stack()
assert_true(s.is_empty(s) == 1, "new stack is empty")

s.push(s, 10)
s.push(s, 20)
s.push(s, 30)
assert_eq(s.size(s), 3, "stack size 3")
assert_eq(s.peek(s), 30, "stack peek top")
assert_eq(s.pop(s), 30, "stack pop top")
assert_eq(s.size(s), 2, "stack size after pop")
assert_eq(s.pop(s), 20, "stack pop next")
assert_eq(s.pop(s), 10, "stack pop last")
assert_true(s.is_empty(s) == 1, "stack empty after pops")

# class method chaining
CLASS Builder
  VAR parts = []

  FN add(self, part) THEN
    append_list(self.parts, part)
    RETURN self
  END

  FN build(self) THEN
    VAR result = ""
    VAR i = 0
    WHILE i < len_of(self.parts) THEN
      IF i > 0 THEN
        result = result + "-"
      END
      result = result + self.parts[i]
      i = i + 1
    END
    RETURN result
  END
END

VAR b = Builder()
VAR result = b.add(b, "hello").add(b, "world").add(b, "arc").build(b)
assert_true(result == "hello-world-arc", "builder chain")

print("test_class_patterns.arc passed\n")
