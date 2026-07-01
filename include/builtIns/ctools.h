#ifndef ARC_CTOOLS_H
#define ARC_CTOOLS_H

#include "../object.h"

Object* builtIn_dl_open(Object** args, size_t argCount);
Object* builtIn_dl_sym(Object** args, size_t argCount); 
Object* builtIn_dl_close(Object** args, size_t argCount);

Object* builtIn_raw_dl_sym(Object** args, size_t argCount);

Object* builtIn_c_func_signature(Object** args, size_t argCount);
Object* builtIn_c_run(Object** args, size_t argCount);

#endif // ARC_CTOOLS_H
