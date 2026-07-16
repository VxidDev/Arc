#ifndef ARC_SYS_H
#define ARC_SYS_H

#include "../object.h"

#if defined(_WIN32)
  #define OS_NAME "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
  #define OS_NAME "MacOS"
#elif defined(__linux__)
  #define OS_NAME "Linux"
#else
  #define OS_NAME "Unknown"
#endif

Object* builtIn_exit(Object** args, size_t argCount);
Object* builtIn_system(Object** args, size_t argCount);
Object* builtIn_getenv(Object** args, size_t argCount);
Object* builtIn_get_os(Object** args, size_t argCount);

#ifndef _WIN32
  Object* builtIn_access(Object** args, size_t argCount);
  Object* builtIn_unlink(Object** args, size_t argCount);
  Object* builtIn_write(Object** args, size_t argCount);

  #define FS_OK 0
  #define FS_NOTFOUND 1 
  #define FS_PERM 2 
  #define FS_UNKNOWN 3
#endif 

#endif // ARC_SYS_H
