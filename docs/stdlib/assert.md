# Assertion Library

The `assert` module provides simple testing utilities for validating program behavior.

## Functions

### `assert_eq(a, b, msg)`
- **Description:** Asserts that `a` and `b` are approximately equal (within an epsilon of `0.0001`). If the condition is not met, a `RuntimeError` is raised with the provided message.

### `assert_true(cond, msg)`
- **Description:** Asserts that the provided condition `cond` is true. If not, a `RuntimeError` is raised with the provided message.
