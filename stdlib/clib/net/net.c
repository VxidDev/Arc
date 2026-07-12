#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>

#include "../../../include/utils.h"
#include "../../../include/error.h"
#include "../../../include/repl/repl.h"
#include "../../../include/c-bridge.h"

#include "./axionetd/include/axionetd.h"
#include "./axionetd/include/memory-pool.h"
#include "./axionetd/include/http.h"
#include "./axionetd/include/router.h"

static CURL* curl = NULL;
static bool initialized = false;

Object* arcNet_request_init(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;
  
  if (curl) {
    return (Object*)initInt(1);
  }
  
  if (!initialized) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    initialized = true;
  }

  curl = curl_easy_init();

  if (!curl) {
    return (Object*)initProgramError("Failed to initialize CURL.");
  }

  return (Object*)initInt(1);
}

typedef struct {
  char *data;
  size_t size;
  size_t capacity;
} ResponseBuffer;

size_t __arcNet_writeCallback(void *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t realsize = size * nmemb;
  ResponseBuffer *mem = (ResponseBuffer*)userdata;

  if (mem->size + realsize + 1 > mem->capacity) {
    size_t newCapacity = mem->capacity + realsize + 4096;
    char *newPtr = realloc(mem->data, newCapacity);
    
    if (!newPtr) {
      fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
      return 0; // Returning 0 signals libcurl to abort the transfer
    }
    
    mem->data = newPtr;
    mem->capacity = newCapacity;
  }

  memcpy(&(mem->data[mem->size]), ptr, realsize);
  mem->size += realsize;
  mem->data[mem->size] = '\0';

  return realsize;
}

Object* arcNet_request_get(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_STRING, 1);
  if (err) return err;

  if (!curl) {
    return (Object*)initProgramError("Curl is not initialized, initialize net first.");
  }
  
  ResponseBuffer buffer = { .data = malloc(4096), .size = 0, .capacity = 4096};

  curl_easy_setopt(curl, CURLOPT_URL, ((String*)args[0])->value);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __arcNet_writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&buffer);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "Failed to perform request: %s", curl_easy_strerror(res));

    return (Object*)initProgramError(buf);
  }

  return (Object*)noCopyInitString(buffer.data, buffer.size);
}

Object* arcNet_request_deinit(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;
  
  if (!curl) {
    return (Object*)initInt(1);
  }

  curl_easy_cleanup(curl);
  curl = NULL;

  curl_global_cleanup();
  initialized = false;

  return (Object*)initInt(1);
}

Object* arcNet_HttpServer(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_STRING, 1); // host 
  if (err) return err;

  err = enforceType(args[1], OBJ_NUMBER_INT, 2); // port 
  if (err) return err;

  err = enforceType(args[2], OBJ_NUMBER_INT, 3); // backlog
  if (err) return err;

  err = enforceType(args[3], OBJ_NUMBER_INT, 4); // workers
  if (err) return err;

  err = enforceType(args[4], OBJ_NUMBER_INT, 5); // logging 
  if (err) return err;

  int64_t logging = ((Number*)args[4])->as.i;

  if (logging != 0 && logging != 1) { // ensure bool
    return (Object*)initProgramError("Expected argument 5 to be a valid bool.");
  }
  
  const char *host = ((String*)args[0])->value;
  const int port = ((Number*)args[1])->as.i;
  const int backlog = ((Number*)args[2])->as.i;
  int workers = ((Number*)args[3])->as.i;

  Axionet* server = initServer(host, port, backlog, workers, logging);

  if (!server) {
    return (Object*)initProgramError("Failed to initialize HTTP server.");
  }
  
  return (Object*)initInt((int64_t)(uintptr_t)server);
}

Object* arcNet_start_server(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  startServer((Axionet*)(uintptr_t)((Number*)args[0])->as.i);
  return (Object*)initInt(0);
}

void __arcNet_axionet_trampoline(AxioRequest *req, AxioResponse *res, axio_MemoryPool *pool, void *userdata) {
  Function* func = (Function*)userdata;
  
  Value args[] = { VAL_OBJ((Object*)initString(req->path, strlen(req->path))) };
  Object* result = callArcFunction(vm, func, args, 1);

  if (vm->err && *vm->err) {
    printf("%s\n", errorAsString(*vm->err));
    freeError(*vm->err);
    *vm->err = NULL;
  }

  if (!result) {
    printf("Route handler returned null.\n");
    return;
  }
  
  if (result->type != OBJ_STRING) {
    printf("Expected route response to be string, received: %s\n", typeofobj(result));
    freeObject(result);
    return;
  }

  HTMLResponse(res, ((String*)result)->value, 200, NULL, 0, pool);
  freeObject(result);
}

Object* arcNet_add_route(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // server ptr 
  if (err) return err;

  err = enforceType(args[1], OBJ_STRING, 2); // path 
  if (err) return err;

  err = enforceType(args[2], OBJ_LIST, 3); // list of methods
  if (err) return err;

  err = enforceType(args[3], OBJ_FUNCTION, 4); // handler
  if (err) return err;

  AxioRoute* route = malloc(sizeof(AxioRoute));

  if (!route) {
    return (Object*)initProgramError("Failed to register route. (Out of memory)");
  }

  void* server = (void*)(uintptr_t)((Number*)args[0])->as.i;
  char* path = stringDup(((String*)args[1])->value);
  
  if (!path) {
    free(route);
    return (Object*)initProgramError("Failed to register route. (Out of memory)");
  }

  List* methodsList = (List*)args[2];
  char** methods = malloc(sizeof(char*) * methodsList->size);

  if (!methods) {
    free(route);
    free(path);
    return (Object*)initProgramError("Failed to register route. (Out of memory)");
  }

  for (size_t i = 0; i < methodsList->size; i++) {
    if (methodsList->objects[i]->type != OBJ_STRING) {
      free(route);
      free(path);

      for (size_t j = 0; j < i; j++) {
        free(methods[j]);
      }

      return (Object*)initProgramError("Methods are supposed to be list of strings.");
    }

    methods[i] = stringDup(((String*)methodsList->objects[i])->value);
  }

  Function* func = (Function*)args[3];

  bool ok = addRoute(server, path, methods, methodsList->size, __arcNet_axionet_trampoline, route, false);

  if (!ok) {
    for (size_t i = 0; i < methodsList->size; i++) free(methods[i]);
    free(methods);
    free(path);
    free(route);
    return (Object*)initProgramError("Failed to register route.");
  }

  route->userdata = func;

  return (Object*)initInt(1);
}
