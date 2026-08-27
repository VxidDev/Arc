# defining a class
CLASS Animal
  VAR name = ""
  VAR sound = ""

  FN init(self, name, sound) THEN
    self.name = name
    self.sound = sound
    RETURN self
  END

  FN speak(self) THEN
    RETURN self.name + " says " + self.sound + "!"
  END
END

# creating instances
VAR dog = Animal()
dog.init(dog, "Dog", "Woof")
print(dog.speak(dog))

VAR cat = Animal()
cat.init(cat, "Cat", "Meow")
print(cat.speak(cat))

# class with computation
CLASS Point
  VAR x = 0
  VAR y = 0

  FN init(self, x, y) THEN
    self.x = x
    self.y = y
    RETURN self
  END

  FN distance_to(self, other) THEN
    VAR dx = self.x - other.x
    VAR dy = self.y - other.y
    RETURN sqrt(dx * dx + dy * dy)
  END

  FN toString(self) THEN
    RETURN "(" + to_string(self.x) + ", " + to_string(self.y) + ")"
  END
END

IMPORT "@stdlib/math.arc"

VAR p1 = Point()
p1.init(p1, 0, 0)

VAR p2 = Point()
p2.init(p2, 3, 4)

print("p1 = " + p1.toString(p1))
print("p2 = " + p2.toString(p2))
print("distance = " + to_string(p1.distance_to(p1, p2)))
