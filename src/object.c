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
  
  #ifndef ARC_EXCLUDE_CTOOLS
  &(NativeModuleEntry){ "__c_tools", initCTools },
  #endif // ARC_EXCLUDE_CTOOLS
  
  &(NativeModuleEntry){ "__random", initRandomModule },

  &(NativeModuleEntry){ "__lib_tools", initLibtools }, NULL 
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
    case OBJ_FILE:
      return (Object*)copyFile((File*)obj);
    default: 
      return obj;
  }
}

void freeObject(Object* obj) {
  if (!obj || obj->isStatic) return;
  
  switch (obj->type) {
    case OBJ_NUMBER_INT:
    case OBJ_NUMBER_FLOAT:
      poolFree(numberPool, obj);
      return;

    case OBJ_STRING:
      String* s = (String*)obj;
      if (s->ownsValue && s->value) free(s->value);
      poolFree(stringPool, obj); 
      return; 
 
    case OBJ_FUNCTION:
      if (((Function*)obj)->chunk) 
        freeChunk(((Function*)obj)->chunk);

      poolFree(functionPool, obj);
      return;

    case OBJ_RETURN: {
      freeObject(((Return*)obj)->value);
      free(obj);
      return;
    }

    case OBJ_FUNCTION_CALL: {
      poolFree(functionCallPool, obj);
      return;
    }
    
    case OBJ_ERROR:
    case OBJ_FILE: 
    case OBJ_CONTINUE:
    case OBJ_MODULE:
    case OBJ_BREAK: {
      free(obj);
      return;
    }

    case OBJ_INSTANCE: {
      Instance* inst = (Instance*)obj;
      if (--inst->base.refCount > 0) break;

      freeTable(inst->fields);
      poolFree(instancePool, inst);
      return;
    }
    
    case OBJ_LIST: {
      List* list = (List*)obj;
      if (--list->base.refCount > 0) return;
      
      for (uint64_t i = 0; i < list->size; i++) {
        Object* elem = list->objects[i];

        if (elem && !elem->isStatic) {
          if (elem->type == OBJ_NUMBER_INT || elem->type == OBJ_NUMBER_FLOAT)
            poolFree(numberPool, elem);
          else if (elem->type == OBJ_STRING) {
            free(((String*)elem)->value);
            poolFree(stringPool, elem);
          } else freeObject(elem);
        }
      }

      free(list->objects);
      free(list);

      return;
    }

    default: return;
  }
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
