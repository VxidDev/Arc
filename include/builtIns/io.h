#ifndef IO_H 
#define IO_H 

#include "../object.h"

Object* builtIn_print(Object** args, size_t argCount);
Object* builtIn_get_input(Object** args, size_t argCount);

#endif // !IO_H 
