#include "../include/object.h"
#include "../include/symbol-table.h"

#include "../include/token.h"

#include <stddef.h>
#include <stdlib.h>

#include "../include/builtIns.h"
#include "../include/mempool.h"

const NativeModuleEntry* stdlibModules[] = {
  &(NativeModuleEntry){ "__mathlib", initMathModule }, 
  &(NativeModuleEntry){ "__sys", initSysModule }, 
  &(NativeModuleEntry){ "__time", initTimeModule }, NULL 
};

Object* copyObject(Object* obj) {
  if (!obj) return NULL;
  
  switch (obj->type) {
    case OBJ_NUMBER_INT:
      return (Object*)copyNumber((Number*)obj);
    case OBJ_NUMBER_FLOAT:
      return (Object*)copyNumber((Number*)obj);
    case OBJ_LIST:
      return (Object*)copyList((List*)obj);
    case OBJ_NATIVE_FUNCTION:
      return (Object*)copyNativeFunction((NativeFunction*)obj);
    case OBJ_STRING:
      return (Object*)copyString((String*)obj);
    case OBJ_MODULE:
    case OBJ_FUNCTION:
    case OBJ_ERROR:
    case OBJ_CONTINUE:
    case OBJ_BREAK:
      return obj;
    case OBJ_FILE:
      return (Object*)copyFile((File*)obj);
    default:
      return NULL;
  }
}

void freeObject(Object* obj) {
  if (!obj) return;

  switch (obj->type) {
    case OBJ_NUMBER_INT:
      Number* n = (Number*)obj;

      if (n->isStatic) return;

      poolFree(numberPool, obj); break;
    
    case OBJ_NUMBER_FLOAT:
      poolFree(numberPool, obj); break;

    case OBJ_STRING: 
      poolFree(stringPool, obj); break;
    
    case OBJ_NATIVE_FUNCTION:
      poolFree(nativeFuncPool, obj); break;

    case OBJ_LIST: {
      List* list = (List*)obj;
      
      if (list->objects) {
        for (uint64_t i = 0; i < list->size; i++) 
          freeObject(list->objects[i]);

        free(list->objects);
      }

      free(list);

      break;
    }
    
    case OBJ_MODULE:
      free(obj);
      break;

    case OBJ_FUNCTION:
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
  }

  return;
}
