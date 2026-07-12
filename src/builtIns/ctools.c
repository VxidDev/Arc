#ifndef ARC_EXCLUDE_CTOOLS

#include "../../include/builtIns/ctools.h"
#include "../../include/utils.h"

#include <inttypes.h>
#include <stdlib.h>

#include <ffi.h>
#include <ffitarget.h>

#include <string.h>

#ifdef _WIN32
#include <windows.h>

typedef HMODULE arcLibHandle;

static inline arcLibHandle arcDlopen(const char *path) {
  return LoadLibraryA(path);
}

static inline void *arcDlsym(arcLibHandle handle, const char *name) {
  return (void *)GetProcAddress(handle, name);
}

static inline int arcDlclose(arcLibHandle handle) {
  return FreeLibrary(handle) ? 0 : -1;
}

#else
#include <dlfcn.h>

typedef void *arcLibHandle;

static inline arcLibHandle arcDlopen(const char *path) {
  return dlopen(path, RTLD_LAZY);
}

static inline void *arcDlsym(arcLibHandle handle, const char *name) {
  return dlsym(handle, name);
}

static inline int arcDlclose(arcLibHandle handle) {
  return dlclose(handle);
}
#endif

typedef struct {
  void *funcPtr;
  ffi_cif cif;
  ffi_type **argSignature;
  int64_t returnType;
  unsigned int nargs;
  bool isInitialized;
} FFICache;

typedef union {
  int64_t i64;
  double d;
  float f;
  void* p;
} FFIResult;

static FFICache last = { .isInitialized = false };

typedef enum C_FFI_TYPES {
  C_INT,
  C_INT_PTR,
  C_FLOAT,
  C_FLOAT_PTR,
  C_DOUBLE,
  C_DOUBLE_PTR,
  C_CHAR,
  C_CHAR_PTR,
  C_VOID_PTR,
  C_VOID
} C_FFI_TYPES;

void* __convert_to_primitive(Object* obj) {
  switch (obj->type) {
    case OBJ_NUMBER_INT: return &((Number*)obj)->as.i;
    case OBJ_NUMBER_FLOAT: return  &((Number*)obj)->as.f;
    case OBJ_STRING: return &((String*)obj)->value;
    case OBJ_LIST: {
      List* list = (List*)obj;
      void** converted = malloc(sizeof(void*) * list->size);
      
      if (!converted) return NULL;

      for (size_t i = 0; i < list->size; i++) {
        converted[i] = __convert_to_primitive(list->objects[i]);
      }

      return converted;
    }
    
    default: return NULL;
  }
}

Object* builtIn_c_run(Object** args, size_t argCount) {
  if (argCount > 3) {
    Object* err = enforceType(args[3], OBJ_NUMBER_INT, 4);
    if (err) return err;
  }

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // ptr 
  if (err) return err;

  err = enforceType(args[1], OBJ_LIST, 2); // signature 
  if (err) return err;

  err = enforceType(args[2], OBJ_LIST, 3); // args 
  if (err) return err;

  bool isVariadic = (argCount > 3);
  int64_t fixedArgsCount = isVariadic ? ((Number*)args[3])->as.i : 0;

  void* funcPtr = (void*)(uintptr_t)(((Number*)args[0])->as.i);
  List* arguments = (List*)args[2];
  List* signature = (List*)args[1];

  unsigned int expectedArgs = (unsigned int)signature->size - 1;
  
  if (arguments->size != expectedArgs) {
    char buf[128];
    
    snprintf(buf, sizeof(buf),
      "FFI call argument count mismatch: signature expects %u args, got %zu",
      expectedArgs, arguments->size);
    
    return (Object*)initProgramError(buf);
  }

  ffi_cif cif;
  ffi_type **local_argSignature = NULL;
  int64_t retType;

  if (!isVariadic && last.isInitialized && last.funcPtr == funcPtr && last.nargs == expectedArgs) {
    cif = last.cif;
    retType = last.returnType;
  } else {
    ffi_type **argSignature = malloc(sizeof(ffi_type*) * signature->size);

    if (!argSignature) {
      return (Object*)initProgramError("Failed to prepare argSignature for ffi_cif. (Out of memory)");
    }

    ffi_type* returnType;
    retType = ((Number*)signature->objects[0])->as.i;

    switch (retType) {
      case C_INT: returnType = &ffi_type_sint; break;
      case C_INT_PTR: returnType = &ffi_type_pointer; break;
      case C_FLOAT: returnType = &ffi_type_float; break;
      case C_FLOAT_PTR: returnType = &ffi_type_pointer; break;      
      case C_DOUBLE: returnType = &ffi_type_double; break;
      case C_DOUBLE_PTR: returnType = &ffi_type_pointer; break;
      case C_CHAR: returnType = &ffi_type_schar; break;
      case C_CHAR_PTR: returnType = &ffi_type_pointer; break;
      case C_VOID_PTR: returnType = &ffi_type_pointer; break;
      case C_VOID: returnType = &ffi_type_void; break;
      default: {
        char buf[256];
        snprintf(buf, sizeof(buf), "Invalid FFI return type signature. (Value %" PRId64 " is not valid)", retType);
        return (Object*)initProgramError(buf);
      }
    }

    for (size_t i = 1; i < signature->size; i++) {
      err = enforceType(signature->objects[i], OBJ_NUMBER_INT, i + 1);

      if (err) {
        free(argSignature);
        return err;
      }

      int64_t type = ((Number*)signature->objects[i])->as.i;

      switch (type) {
        case C_INT: argSignature[i - 1] = &ffi_type_sint; break;
        case C_INT_PTR: argSignature[i - 1] = &ffi_type_pointer; break;
        case C_FLOAT: argSignature[i - 1] = &ffi_type_float; break;
        case C_FLOAT_PTR: argSignature[i - 1] = &ffi_type_pointer; break;      
        case C_DOUBLE: argSignature[i - 1] = &ffi_type_double; break;
        case C_DOUBLE_PTR: argSignature[i - 1] = &ffi_type_pointer; break;
        case C_CHAR: argSignature[i - 1] = &ffi_type_schar; break;
        case C_CHAR_PTR: argSignature[i - 1] = &ffi_type_pointer; break;
        case C_VOID_PTR: argSignature[i - 1] = &ffi_type_pointer; break;
        default: {
          char buf[256];
          snprintf(buf, sizeof(buf), "Invalid FFI function signature. (Value %" PRId64 " is not valid)", type);
          return (Object*)initProgramError(buf);
        }
      }
    }

    unsigned int totalArgsCount = signature->size - 1;

    if (isVariadic) {
      if (ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, (unsigned int)fixedArgsCount, totalArgsCount, returnType, argSignature) != FFI_OK) {
        free(argSignature);
        return (Object*)initProgramError("Failed to prepare variadic ffi_cif.");
      }

      local_argSignature = argSignature;
    } else {  
      if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, signature->size - 1, returnType, argSignature) != FFI_OK) {
        free(argSignature);
        return (Object*)initProgramError("Failed to prepare ffi_cif.");
      }

      if (last.isInitialized && last.argSignature) {
        free(last.argSignature);
      }

      last.cif = cif;
      last.funcPtr = funcPtr;
      last.argSignature = argSignature;
      last.returnType = retType;
      last.isInitialized = true;
      last.nargs = totalArgsCount;
    }
  }
  
  void** values = malloc(sizeof(void*) * arguments->size);
  
  if (!values) {
    return (Object*)initProgramError("Failed to prepare values for ffi_call. (Out of memory)");
  }

  for (size_t i = 0; i < arguments->size; i++) { 
    void* addr = __convert_to_primitive(arguments->objects[i]);
    
    if (!addr) {
      free(values);
      return (Object*)initProgramError("Failed to prepare values for ffi_call. (Out of memory)");
    }

    values[i] = addr;
  }
  
  FFIResult result;
  ffi_call(&cif, FFI_FN(funcPtr), &result, values);

  free(values);
  if (local_argSignature) free(local_argSignature);

  switch (isVariadic ? retType : last.returnType) {
    case C_INT: return (Object*)initInt(result.i64);
    case C_INT_PTR: return (Object*)initInt((int64_t)(uintptr_t)result.p);
    case C_FLOAT: return (Object*)initFloat(result.f);
    case C_FLOAT_PTR: return (Object*)initInt((int64_t)(uintptr_t)result.p);      
    case C_DOUBLE: return (Object*)initFloat(result.d);
    case C_DOUBLE_PTR: return (Object*)initInt((int64_t)(uintptr_t)result.p);
    case C_CHAR: return (Object*)initString((char[]){result.i64, '\0'}, 2);
    case C_CHAR_PTR: return (Object*)initInt((int64_t)(uintptr_t)result.p);
    case C_VOID_PTR: return (Object*)initInt((int64_t)(uintptr_t)result.p);
    case C_VOID: return (Object*)initNull();
  }

  return (Object*)initNull();
}

void ctoolsCleanup(void) {
  if (last.isInitialized && last.argSignature) {
    free(last.argSignature);
    last.argSignature = NULL;
    last.isInitialized = false;
  }
}

Object* builtIn_dl_open(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_STRING, 1);

  if (err) {
    return err;
  }

  String* path = (String*)args[0];

  arcLibHandle handle = arcDlopen(path->value);

  if (!handle) {
    return (Object*)initProgramError("Failed to open shared object.");
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

  arcDlclose((void*)((uintptr_t)handle->as.i));

  return (Object*)initInt(1);
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

  NativeFunc func = (NativeFunc)arcDlsym((void*)handle, name->value);

  return (Object*)initNativeFunction(name->value, func, paramCount->as.i, isVariant->as.i);
}

Object* builtIn_raw_dl_sym(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  err = enforceType(args[1], OBJ_STRING, 2);
  if (err) return err;
 
  Number* handleObj = (Number*)args[0];
  uintptr_t handle = (uintptr_t)handleObj->as.i;

  String* name = (String*)args[1];
  void* func = arcDlsym((void*)handle, name->value);

  return (Object*)initInt((int64_t)(uintptr_t)func);
}

Object* builtIn_c_func_signature(Object** args, size_t argCount) {
  Object** copiedObjects = malloc(sizeof(Object*) * argCount);

  if (!copiedObjects) {
    return NULL;
  }

  for (size_t i = 0; i < argCount; i++) {
    Object* err = enforceType(args[i], OBJ_NUMBER_INT, i + 1);
    
    if (!err) {
      Number* n = (Number*)args[i];

      if (n->as.i < C_INT || n->as.i > C_VOID_PTR) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Argument %zu is not a valid C_FFI_TYPES enum value.", i + 1);
        err = (Object*)initProgramError(buf);
      }
    }

    if (err) {
      for (size_t j = 0; j < i; j++) {
        freeObject(copiedObjects[j]);
      }

      free(copiedObjects);
      return err;
    }

    copiedObjects[i] = copyObject(args[i]);
  }

  List* list = initList(copiedObjects, argCount, argCount);
  free(copiedObjects);

  return (Object*)list;
}

Object* builtIn_string_at(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;
  
  void* ptr = (void*)(uintptr_t)((Number*)args[0])->as.i;
  char* s = (char*)ptr;

  return (Object*)noCopyInitString(s, strlen(s));
}

Object* builtIn_int_at(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  void* ptr = (void*)(uintptr_t)((Number*)args[0])->as.i;
  int i = *(int*)ptr;

  return (Object*)initInt(i);
}

Object* builtIn_pointer_at(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  void* ptr = (void*)(uintptr_t)((Number*)args[0])->as.i;
  void* deref = *(void**)ptr;

  return (Object*)initInt((int64_t)(uintptr_t)deref);
}

#endif
