#ifndef ARC_SYS_H
#define ARC_SYS_H

#include "../object.h"

Object* builtIn_exit(Object** args, size_t argCount);

#ifndef _WIN32
  Object* builtIn_access(Object** args, size_t argCount);
  Object* builtIn_unlink(Object** args, size_t argCount);

  #define FS_OK 0
  #define FS_NOTFOUND 1 
  #define FS_PERM 2 
  #define FS_UNKNOWN 3
#endif 

#endif // ARC_SYS_H
