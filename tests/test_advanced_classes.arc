IMPORT "@stdlib/assert.arc"

# basic class with fields
CLASS Dog
  VAR name = "unknown"
  VAR age = 0

  FN bark(self) THEN
    RETURN self.name + " says woof"
  END

  FN get_age(self) THEN
    RETURN self.age
  END
END

VAR d = Dog()
assert_true(d.name == "unknown", "default field value")
d.name = "Rex"
assert_true(d.name == "Rex", "field assignment")
assert_true(d.bark(d) == "Rex says woof", "method call")
assert_eq(d.get_age(d), 0, "method returning field")

# class with multiple instances
VAR d1 = Dog()
VAR d2 = Dog()
d1.name = "Buddy"
d2.name = "Max"
assert_true(d1.name == "Buddy", "instance 1 name")
assert_true(d2.name == "Max", "instance 2 name")

# class with computed methods
CLASS Calculator
  VAR value = 0

  FN add(self, n) THEN
    self.value = self.value + n
    RETURN self
  END

  FN multiply(self, n) THEN
    self.value = self.value * n
    RETURN self
  END

  FN get(self) THEN
    RETURN self.value
  END
END

VAR calc = Calculator()
calc.add(calc, 10)
assert_eq(calc.get(calc), 10, "calc add 10")
calc.multiply(calc, 3)
assert_eq(calc.get(calc), 30, "calc multiply 3")
calc.add(calc, 5)
assert_eq(calc.get(calc), 35, "calc add 5 after multiply")

# class with list field
CLASS Playlist
  VAR songs = []

  FN add_song(self, song) THEN
    append_list(self.songs, song)
    RETURN self
  END

  FN count(self) THEN
    RETURN len_of(self.songs)
  END

  FN get_song(self, i) THEN
    RETURN self.songs[i]
  END
END

VAR pl = Playlist()
pl.add_song(pl, "Song A")
pl.add_song(pl, "Song B")
pl.add_song(pl, "Song C")
assert_eq(pl.count(pl), 3, "playlist count")
assert_true(pl.get_song(pl, 0) == "Song A", "playlist first song")
assert_true(pl.get_song(pl, 2) == "Song C", "playlist last song")

# class method modifying another field
CLASS BankAccount
  VAR balance = 0

  FN deposit(self, amount) THEN
    self.balance = self.balance + amount
    RETURN self
  END

  FN withdraw(self, amount) THEN
    IF amount > self.balance THEN
      RETURN RuntimeError("Insufficient funds")
    END
    self.balance = self.balance - amount
    RETURN self
  END

  FN get_balance(self) THEN
    RETURN self.balance
  END
END

VAR acc = BankAccount()
acc.deposit(acc, 100)
assert_eq(acc.get_balance(acc), 100, "deposit 100")
acc.withdraw(acc, 30)
assert_eq(acc.get_balance(acc), 70, "withdraw 30")

TRY
  acc.withdraw(acc, 200)
  assert_true(0, "should not reach here")
CATCH e THEN
  assert_true(1, "insufficient funds caught")
END

assert_eq(acc.get_balance(acc), 70, "balance unchanged after failed withdraw")

print("test_advanced_classes.arc passed\n")
