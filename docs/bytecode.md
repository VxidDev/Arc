# Arc Bytecode Reference

This document lists all opcodes used by the Arc Virtual Machine, their stack effects, and their behavior. Opcodes are defined in `include/compiler.h` and implemented in `src/vm.c`.

## Data Movement

### OP_LOAD_CONST
- **Stack**: `... -> ..., value`
- **Operand**: 1-byte index into the constant pool.
- **Description**: Pushes a constant (Number, String, or Function) onto the stack.

### OP_LOAD_VAR
- **Stack**: `... -> ..., value`
- **Operand**: 1-byte index to interned string (name).
- **Description**: Looks up a variable in the current symbol table and pushes its value.

### OP_STORE_VAR
- **Stack**: `..., value -> ..., value` (peeks)
- **Operand**: 1-byte index to interned string (name).
- **Description**: Assigns the top stack value to a variable in the current symbol table.

### OP_LOAD_LOCAL
- **Stack**: `... -> ..., value`
- **Operand**: 1-byte stack slot index.
- **Description**: Pushes the value of a local variable from the current call frame.

### OP_STORE_LOCAL
- **Stack**: `..., value -> ..., value` (peeks)
- **Operand**: 1-byte stack slot index.
- **Description**: Assigns the top stack value to a local variable slot.

### OP_POP
- **Stack**: `..., value -> ...`
- **Description**: Discards the top value on the stack.

## Arithmetic & Logic

### OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_POW
- **Stack**: `..., a, b -> ..., (a op b)`
- **Description**: Binary arithmetic operations. `OP_DIV` raises a "Division by zero" error if `b` is 0.

### OP_EQ, OP_NE, OP_LT, OP_GT, OP_LTE, OP_GTE
- **Stack**: `..., a, b -> ..., (0 or 1)`
- **Description**: Comparison operations. Results in an integer `1` (true) or `0` (false).

### OP_AND, OP_OR
- **Stack**: `..., a, b -> ..., (0 or 1)`
- **Description**: Logical AND/OR operations.

### OP_NEG
- **Stack**: `..., a -> ..., -a`
- **Description**: Unary arithmetic negation.

### OP_NOT
- **Stack**: `..., a -> ..., !a`
- **Description**: Logical NOT.

## Control Flow

### OP_JUMP
- **Operand**: 2-byte signed offset.
- **Description**: Unconditionally adds the offset to the Instruction Pointer (`ip`).

### OP_JUMP_IF_FALSE
- **Stack**: `..., condition -> ...` (pops)
- **Operand**: 2-byte signed offset.
- **Description**: Jumps if the condition is `0`.

### OP_FOR_PREP
- **Stack**: `..., iterable -> ..., iterable, length, index(0)`
- **Description**: Prepares a `FOR` loop by pushing the iterable's length and an initial index onto the stack.

### OP_FOR_ITER
- **Stack**: `..., iterable, length, index -> ..., iterable, length, index, item` (if continuing)
- **Operand**: 2-byte signed offset (to exit).
- **Description**: Checks if `index < length`. If so, pushes the next item and increments the index. If not, jumps to exit.

### OP_BREAK / OP_CONTINUE
- **Description**: Internal markers returned to the VM loop to handle loop control flow.

## Functions & Execution

### OP_CALL
- **Stack**: `..., callee, arg1, ..., argN -> ..., result`
- **Operand**: 1-byte argument count `N`.
- **Description**: Invokes an Arc function or Native function.

### OP_RETURN
- **Stack**: `..., result -> (caller stack)`
- **Description**: Returns from the current function frame with the top stack value.

### OP_PROPERTY_ACCESS
- **Stack**: `..., instance -> ..., value`
- **Operand**: 1-byte index to interned string (property name).
- **Description**: Accesses a property value from an instance.

### OP_PROPERTY_SET
- **Stack**: `..., instance, value -> ..., value`
- **Operand**: 1-byte index to interned string (property name).
- **Description**: Sets a property value on an instance.

## Collections & Indexing

### OP_BUILD_LIST
- **Stack**: `..., item1, ..., itemN -> ..., [list]`
- **Operand**: 1-byte count `N`.
- **Description**: Creates a new list containing the top `N` stack items.

### OP_INDEX_GET
- **Stack**: `..., target, index -> ..., value`
- **Description**: Retrieves an element from a String or List at the given index.

### OP_INDEX_SET
- **Stack**: `..., target, index, value -> ..., 1`
- **Description**: Sets an element in a String or List. Pushes `1` on success.

### OP_STORE_INDEX
- **Stack**: `..., index, value -> ..., 1`
- **Operand**: 1-byte index to interned string (target name).
- **Description**: Specialized assignment for list/string elements: `target[index] = value`.

## Miscellaneous

### OP_TRY_PUSH
- **Operand**: 2-byte signed offset (to catch block).
- **Description**: Pushes the catch block address onto the `tryStack`.

### OP_TRY_POP
- **Description**: Removes the top entry from the `tryStack`.

### OP_IMPORT
- **Operand**: 1-byte index to constant string (path).
- **Description**: Loads and executes an external file or native module.

### OP_HALT
- **Description**: Stops the VM execution loop.
