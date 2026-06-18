#ifndef ARC_SYS_H
#define ARC_SYS_H

#include "../object.h"

Object* builtIn_exit(Object** args, size_t argCount);

#ifndef _WIN32
  Object* builtIn_access(Object** args, size_t argCount);
#endif 

#endif // ARC_SYS_H
