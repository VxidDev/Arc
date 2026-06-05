#ifndef TYPING_H
#define TYPING_H

#include "../object.h"

Object* builtIn_typeof(Object** args, size_t argCount);
Object* builtIn_to_int(Object** args, size_t argCount);
Object* builtIn_to_string(Object** args, size_t argCount);

#endif // TYPING_H 
