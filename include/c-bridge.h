#ifndef C_BRIDGE_H
#define C_BRIDGE_H

#include "../include/object.h"
#include "../include/vm.h"

Object *callArcFunction(VM *vm, Function *func, Value *args, int argCount);

#endif // C_BRIDGE_H
