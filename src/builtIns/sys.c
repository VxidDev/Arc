#include "../../include/builtIns/sys.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>

#ifndef _WIN32 
  #include <unistd.h> // TODO: windows support 
#endif

Object* builtIn_exit(Object** args, size_t argCount) {
  (void)argCount;

  Object* exitCode = args[0];

  if (exitCode->type != OBJ_NUMBER_INT) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected object of type 'int', received '%s'.", typeofobj(exitCode));

    return (Object*)initProgramError(buf);
  }

  char buf[256];

  snprintf(buf, sizeof(buf), "@%" PRId64 "\n", ((Number*)exitCode)->as.i);

  return (Object*)initProgramError(buf); // will exit cleanly
}

#ifndef _WIN32 

Object* builtIn_access(Object** args, size_t argCount) {
  (void)argCount;

  Object* path = args[0];
  Object* mode = args[1];

  if (path->type != OBJ_STRING) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected object of type 'string', received '%s'.", typeofobj(path));
    return (Object*)initProgramError(buf);
  }

  if (mode->type != OBJ_NUMBER_INT) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected object of type 'int', received '%s'.", typeofobj(mode));
    return (Object*)initProgramError(buf);
  }

  String* p = (String*)path;
  Number* m = (Number*)mode;

  return (Object*)initInt(access(p->value, m->as.i));
}

Object* builtIn_unlink(Object** args, size_t argCount) {
  (void)argCount;

  Object* path = args[0];

  if (path->type != OBJ_STRING) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected object of type 'string', received '%s'.", typeofobj(path));
    return (Object*)initProgramError(buf);
  }

  String* p = (String*)path;
  
  if (unlink(p->value) == 0) {
    return (Object*)initInt(FS_OK); 
  }
  
  switch (errno) {
    case ENOENT: 
      return (Object*)initInt(FS_NOTFOUND);

    case EACCES: 
    case EPERM: 
      return (Object*)initInt(FS_PERM);

    default: 
      return (Object*)initInt(FS_UNKNOWN);
  }
}

#endif 
