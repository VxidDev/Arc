#include "../include/object.h"
#include "../include/symbol-table.h"
#include "../include/vm.h"

#include "../include/token.h"
#include "../include/utils.h"
#include "../include/repl/repl.h"

#include <stddef.h>
#include <stdlib.h>

#include "../include/builtIns.h"
#include "../include/mempool.h"

const NativeModuleEntry* stdlibModules[] = {
  &(NativeModuleEntry){ "__mathlib", initMathModule }, 
  &(NativeModuleEntry){ "__sys", initSysModule }, 
  &(NativeModuleEntry){ "__time", initTimeModule }, 
  &(NativeModuleEntry){ "__c_tools", initCTools }, NULL 
};

static Object nullSingleton = { .type = OBJ_NULL, .isStatic = true };

Object* initNull(void) {
  return &nullSingleton;
}

Object* copyObject(Object* obj) {
  if (!obj) return NULL;
  if (obj->isStatic) return obj;
  
  switch (obj->type) {
    case OBJ_NUMBER_INT:
    case OBJ_NUMBER_FLOAT:
      return (Object*)copyNumber((Number*)obj);
    case OBJ_STRING:
      return (Object*)copyString((String*)obj);
    case OBJ_LIST:
    case OBJ_INSTANCE:
      obj->refCount++;
      return obj;
    case OBJ_MODULE:
    case OBJ_FUNCTION:
    case OBJ_NATIVE_FUNCTION:
    case OBJ_ERROR:
    case OBJ_CONTINUE:
    case OBJ_BREAK:
    case OBJ_CLASS:
    case OBJ_NULL:
      return obj;
    case OBJ_FILE:
      return (Object*)copyFile((File*)obj);
    default: return NULL;
  }
}

void freeObject(Object* obj) {
  if (!obj || obj->isStatic) {
    if (_DEBUG) printf("[debug] skipping freeing object. Reason: %s%s\n", !obj ? "object is null" : "object is static, type of object: ", !obj ? "" : typeofobj(obj));
    return;
  }

  switch (obj->type) {
    case OBJ_NUMBER_INT:
    case OBJ_NUMBER_FLOAT:
      poolFree(numberPool, obj); break;

    case OBJ_STRING:
      String* s = (String*)obj;
      if (s->value) free(s->value);
      poolFree(stringPool, obj); 
      break; 
 
    case OBJ_MODULE:
      free(obj);
      break;

    case OBJ_FUNCTION:
      Function* func = (Function*)obj;
      if (_DEBUG) printf("[debug] freeing function. | name = %s\n", func->name);

      if (!func->chunk && !func->body) break;

      if (func->chunk) freeChunk(func->chunk);

      poolFree(functionPool, obj);
      break;

    case OBJ_RETURN: {
      Return* ret = (Return*)obj;

      freeObject(ret->value);
      free(ret);

      break;
    }

    case OBJ_ERROR: {
      ProgramError* err = (ProgramError*)obj;

      free(err);

      break;
    }

    case OBJ_FUNCTION_CALL: {
      // FunctionCall* fncall = (FunctionCall*)obj;
      
      /*
      if (fncall->args) {
        for (size_t i = 0; i < fncall->argCount; i++) {
          freeObject(fncall->args[i]);
        }
      }
      */

      // if (fncall->env) freeTable(fncall->env);
      poolFree(functionCallPool, obj);

      break;
    }

    case OBJ_FILE: {
      File* file = (File*)obj;
      
      free(file);

      break;
    }

    case OBJ_BREAK: {
      free(obj);

      break;
    }

    case OBJ_CONTINUE: {
      free(obj);

      break;
    }

    case OBJ_INSTANCE: {
      Instance* instance = (Instance*)obj;
      if (--instance->base.refCount > 0) break;

      freeTable(instance->fields);
      poolFree(instancePool, instance);

      break;
    }
    
    case OBJ_LIST: {
      List* list = (List*)obj;
      if (--list->base.refCount > 0) break;

      for (uint64_t i = 0; i < list->size; i++) {
        freeObject(list->objects[i]);
      }

      free(list->objects);
      free(list);

      break;
    }  
    
    case OBJ_NULL:
    case OBJ_NATIVE_FUNCTION:
    case OBJ_CLASS: {
       break;
    }
  }

  return;
}

// only used for full program teardown
void forceFreeObject(Object* obj) {
  if (!obj) return;

  if (obj->type == OBJ_INSTANCE) {
    Instance* inst = (Instance*)obj;
    if (inst->fields) freeTable(inst->fields);
    poolFree(instancePool, inst);
    return;
  }

  if (obj->type == OBJ_LIST) {
    List* list = (List*)obj;

    for (uint64_t i = 0; i < list->size; i++)
      forceFreeObject(list->objects[i]);

    free(list->objects);
    free(list);

    return;
  }

  if (obj->type == OBJ_NATIVE_FUNCTION) {
    poolFree(nativeFuncPool, obj);
    return;
  }
  
  freeValue(VAL_OBJ(obj));
}
