#include "../../include/builtIns/sys.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>

#ifndef _WIN32 
  #include <unistd.h> // TODO: windows support 
#endif

Object* builtIn_exit(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  char buf[256];

  snprintf(buf, sizeof(buf), "@%" PRId64 "\n", ((Number*)args[0])->as.i);

  return (Object*)initProgramError(buf); // will exit cleanly
}

Object* builtIn_system(Object** args, size_t argCount) {
  (void)argCount;
  
  Object* err = enforceType(args[0], OBJ_STRING, 1);
  if (err) return err;

  const char* cmd = ((String*)args[0])->value;
  return (Object*)initInt(system(cmd));
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

Object* builtIn_write(Object** args, size_t argCount) {
  (void)argCount;
  
  Object* fd = args[0];
  Object* buf = args[1];
  Object* len = args[2];
  
  if (fd->type != OBJ_NUMBER_INT) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'int', received '%s'.", typeofobj(fd));
    return (Object*)initProgramError(buf);
  }

  if (buf->type != OBJ_STRING) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 2 to be object of type 'string', received '%s'.", typeofobj(fd));
    return (Object*)initProgramError(buf);
  }

  if (len->type != OBJ_NUMBER_INT) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 3 to be object of type 'int', received '%s'.", typeofobj(fd));
    return (Object*)initProgramError(buf);
  }
  
  Number* fileDesc = (Number*)fd;
  String* buffer = (String*)buf;
  Number* length = (Number*)len;

  int res = write(fileDesc->as.i, buffer->value, length->as.i);

  return (Object*)initInt(res);
}

#endif

Object* builtIn_getenv(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_STRING, 1); // key
  if (err) return err;

  char *key = ((String*)args[0])->value;
  
  char *val = getenv(key);

  if (!val) {
    val = "";
  }

  return (Object*)initString(val, strlen(val));
}

Object* builtIn_get_os(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;

  return (Object*)initString(OS_NAME, strlen(OS_NAME));
}
