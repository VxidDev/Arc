#include "../include/object.h"
#include "../include/symbol-table.h"

#include "../include/token.h"

#include <stddef.h>
#include <stdlib.h>

#include "../include/builtIns.h"

const NativeModuleEntry* stdlibModules[] = {
  &(NativeModuleEntry){ "__mathlib", initMathModule }, NULL
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
    case OBJ_FUNCTION:
      return (Object*)copyFunction((Function*)obj);
    case OBJ_NATIVE_FUNCTION:
      return (Object*)copyNativeFunction((NativeFunction*)obj);
    case OBJ_STRING:
      return (Object*)copyString((String*)obj);
    case OBJ_MODULE:
      return obj;
    case OBJ_ERROR:
      return obj;
    case OBJ_FILE:
      return (Object*)copyFile((File*)obj);
    case OBJ_BREAK:
      return obj;
    case OBJ_CONTINUE:
      return obj;
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

      free(obj); break;
    
    case OBJ_NUMBER_FLOAT:
      free(obj); break;

    case OBJ_STRING: {
      String* str = (String*)obj;

      if (str->value) free(str->value);
      free(str);

      break;
    }

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

    case OBJ_FUNCTION: {
      Function* func = (Function*)obj;

      if (func->params) {
        for (size_t i = 0; i < func->paramCount && func->params[i]; i++)
          free(func->params[i]);

        free(func->params);
      }

      if (func->name) free(func->name);

      free(func);

      break;
    }

    case OBJ_NATIVE_FUNCTION: {
      NativeFunction* nativeFunc = (NativeFunction*)obj;

      if (nativeFunc->name) free(nativeFunc->name);
      free(nativeFunc);

      break;
    }

    case OBJ_MODULE: {
      Module* module = (Module*)obj;

      if (module->astTree) freeAST(module->astTree);
      if (module->tokens) freeTokens(module->tokens, module->tokenAmount);
      if (module->parser) free(module->parser);
      
      if (module->lexer) {
        free(module->lexer->filename);
        freeLexer(module->lexer);
      }
      
      free(module);

      break;
    }

    case OBJ_RETURN: {
      Return* ret = (Return*)obj;

      freeObject(ret->value);
      free(ret);

      break;
    }

    case OBJ_ERROR: {
      ProgramError* err = (ProgramError*)obj;

      if (err->details) free(err->details);
      free(err);

      break;
    }

    case OBJ_FUNCTION_CALL: {
      FunctionCall* fncall = (FunctionCall*)obj;

      if (fncall->args) {
        for (size_t i = 0; i < fncall->argCount; i++) {
          freeObject(fncall->args[i]);
        }

        free(fncall->args);
      }

      if (fncall->env) freeTable(fncall->env);
      free(fncall);

      break;
    }

    case OBJ_FILE: {
      File* file = (File*)obj;

      if (file->fname) free(file->fname);
      if (file->fmod) free(file->fmod);
      
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
