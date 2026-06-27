# Arc Object System

The Arc Object System is the foundation of the Arc VM, managing all data types, functions, and control structures within the runtime.

## High-Level Overview

All data in Arc, from primitive numbers to complex objects like lists, modules, and instances, is represented by a common `Object` structure. The VM relies on this uniform structure to perform operations, manage memory, and handle garbage collection.

## Core Structure

```c
typedef struct Object {
  ObjType type;
  bool isStatic;
} Object;
```

- `type`: An enumeration (`ObjType`) identifying the object type (e.g., `OBJ_NUMBER_INT`, `OBJ_STRING`, `OBJ_LIST`).
- `isStatic`: A flag indicating if the object should be considered permanent and not subject to standard garbage collection.

## Memory Management

Arc utilizes an `isStatic` flag to manage object lifecycles.
- Objects with `isStatic = true` are expected to persist throughout the lifetime of the program.
- Standard objects should be managed via the `freeObject` or `forceFreeObject` functions.
- Every object initialization function (e.g., `initInt`, `initString`, `initList`) returns a pointer to a heap-allocated object. It is the responsibility of the caller to ensure these objects are appropriately freed when no longer needed to prevent memory leaks.

## API Reference

### Initialization Functions

These functions initialize objects of various types.

#### `initInt`
- **Description:** Creates a new integer object.
- **Parameters:** `int64_t value`
- **Return Value:** `Number*` (pointer to a newly allocated `Number` object).
- **Usage Example:**
  ```c
  Number* myInt = initInt(42);
  ```

#### `initString`
- **Description:** Creates a new string object.
- **Parameters:** `char *value`, `uint64_t len`
- **Return Value:** `String*`
- **Usage Example:**
  ```c
  String* myStr = initString("Hello, Arc!", 11);
  ```

### Lifecycle Functions

#### `freeObject`
- **Description:** Decrements reference counts or triggers appropriate cleanup based on the object type.
- **Parameters:** `Object* obj`
- **Return Value:** `void`

#### `forceFreeObject`
- **Description:** Immediately deallocates the object.
- **Parameters:** `Object* obj`
- **Return Value:** `void`
