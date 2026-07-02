#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>

#include "../../../include/utils.h"
#include "./axionetd/include/axionetd.h"

static CURL* curl = NULL;

Object* arcNet_request_init(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;
  
  if (curl) {
    return (Object*)initInt(1);
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
