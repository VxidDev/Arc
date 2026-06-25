#include "../../include/builtIns/ctools.h"
#include "../../include/utils.h"

#include <inttypes.h>
#include <dlfcn.h>

Object* builtIn_dl_open(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_STRING, 1);

  if (err) {
    return err;
  }

  err = enforceType(args[1], OBJ_NUMBER_INT, 2);

  if (err) {
    return err;
  }

  String* path = (String*)args[0];
  Number* mode = (Number*)args[1];

  void* handle = dlopen(path->value, mode->as.i);

  if (!handle) {
    char *dlErr = dlerror();
    return (Object*)initProgramError(dlErr ? dlErr : "Unknown error during opening .so");
  }

  return (Object*)initInt((uintptr_t)handle);
}

Object* builtIn_dl_close(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  Number* handle = (Number*)args[0];

  if (!handle) {
    return (Object*)initProgramError("Invalid handle.");
  }

  dlclose((void*)((uintptr_t)handle->as.i));

  return (Object*)initNull();
}

Object* builtIn_dl_sym(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  err = enforceType(args[1], OBJ_STRING, 2);
  if (err) return err;

  err = enforceType(args[2], OBJ_NUMBER_INT, 3);
  if (err) return err;

  err = enforceType(args[3], OBJ_NUMBER_INT, 4);
  if (err) return err;
  
  Number* handleObj = (Number*)args[0];
  uintptr_t handle = (uintptr_t)handleObj->as.i;

  String* name = (String*)args[1];
  Number* paramCount = (Number*)args[2];
  Number* isVariant = (Number*)args[3];

  if (isVariant->as.i != 0 && isVariant->as.i != 1) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 4 to be an integer of values '0' or '1', received '%" PRId64 "'", isVariant->as.i);
    return (Object*)initProgramError(buf);
  }

  NativeFunc func = (NativeFunc)dlsym((void*)handle, name->value);

  return (Object*)initNativeFunction(name->value, func, paramCount->as.i, isVariant->as.i);
}
