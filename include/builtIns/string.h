#ifndef ARC_STRING_H
#define ARC_STRING_H

#include "../object.h"

Object* builtIn_split_string(Object** args, size_t argCount);
Object* builtIn_append_char(Object** args, size_t argCount);
Object* builtIn_string_buffer(Object** args, size_t argCount);
Object* builtIn_string_finish(Object** args, size_t argCount);

#endif // ARC_STRING_H
