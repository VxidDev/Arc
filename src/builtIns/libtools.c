#include "../../include/builtIns/libtools.h"

#include <string.h>

Object* builtIn_stdlib_path(Object** args, size_t argCount) {
  (void)argCount;
  (void)args;

  return (Object*)initString(ARC_LIB_DIR, strlen(ARC_LIB_DIR)); 
}
