# Arc

Arc is a small programming language project written in C, currently focused on building a clean and reliable lexical analysis system (lexer), REPL, and structured error reporting.

The long-term goal is to evolve Arc into a minimal interpreter or compiler with a full frontend pipeline (lexer → parser → AST → evaluation).

---

## ✨ Current Features

- Interactive REPL (`src/repl`)
- Lexer for arithmetic expressions
- Integer and floating-point number support
- Basic operators:
  - `+`, `-`, `*`, `/`
  - parentheses `(` `)`
- Token system (`include/token.h`)
- Error handling system with position tracking:
  - file name
  - line number
  - column number
- Memory-safe design

---

## 💡 Example

```

Arc > 123 + 45
[INT:123, PLUS, INT:45]

Arc > s
[ ARC - ERROR ] Illegal Character: 's'
File stdin, line 1, column 0

```

---

## 🧠 Design Philosophy

Arc is intentionally built with:

- Minimal dependencies (pure C)
- Clear separation of components:
  - lexer
  - tokens
  - errors
  - position tracking
  - REPL layer
- Debug-friendly architecture

---

## 🏗️ Project Structure

```

include/
├── error.h
├── lexer.h
├── position.h
├── token.h
├── utils.h
└── repl/
└── input.h

src/
├── error.c
├── lexer.c
├── position.c
├── token.c
├── utils.c
└── repl/
├── input.c
└── main.c

````

---

## 🔧 Build

Compile the project using:

```bash
make
````

Run the REPL:

```bash
./arc
```

---

## 🧪 Memory Safety

Arc is tested with Valgrind to ensure:

* No memory leaks
* No invalid reads/writes
* Proper ownership handling

```bash
valgrind --leak-check=full --show-leak-kinds=all ./arc
```

---

## 🚧 Roadmap

* [x] Lexer
* [x] Token system
* [x] REPL
* [x] Error reporting with position tracking
* [ ] Identifiers (variables)
* [ ] Keywords
* [ ] Parser (AST generation)
* [ ] Expression evaluation
* [ ] Basic interpreter runtime
* [ ] Bytecode VM (long-term goal)

---

## 📌 Notes

Arc is still early-stage and evolving quickly. The current focus is correctness, memory safety, and building a stable foundation for parsing and evaluation.

---

## 📜 License

GPL-3.0 License
