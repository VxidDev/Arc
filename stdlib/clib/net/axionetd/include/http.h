#ifndef AXIONETD_REQUEST_H
#define AXIONETD_REQUEST_H

#include "axionetd.h"
#include "config.h"
#include "memory-pool.h"
#include <stdalign.h>
#include <stdbool.h>
#include <yyjson.h>

typedef struct AxioHeader {
    char *key, *value;
    size_t klen, vlen;
} AxioHeader;

typedef struct AxioQueryParam {
    char key[AXIO_MAX_QUERY_KEY_LEN], value[AXIO_MAX_QUERY_VALUE_LEN];
    int klen, vlen;
} AxioQueryParam;

typedef struct AxioRequest { 
    char *raw; // Raw request body

    char path[AXIO_MAX_PATH]; // Requested path, e.g "/"
    char method[AXIO_MAX_METHOD]; // Method e.g "GET"

    char *queryString;
    AxioQueryParam *queryParams;
    int queryParamAmount;

    AxioHeader headers[AXIO_MAX_HEADERS]; // Populated after calling AxioRequest_getHeaders
    int headerAmount;

    char contentType[64];
    bool contentTypeSet;

    yyjson_doc *json;
    yyjson_val *jsonRoot;
} AxioRequest;

typedef struct AxioResponse {
    char *response; // HTTP String respons
    int status;
    unsigned long len;
} AxioResponse;

typedef struct AxioConnection {
    int fd;
    AxioState state;
    
    char *response;
    unsigned long total;
    unsigned long sent;

    char *readBuffer;
} AxioConnection;

bool AxioRequest_parseJSON(AxioRequest* request);
bool parseRequest(AxioRequest* request, char *buf, MemoryPool *queryPool);
void initResponse(AxioResponse* resp, const char* body, const int status, AxioHeader* headers, int headerCount, MemoryPool *responsePool);
void HTMLResponse(AxioResponse* resp, const char* body, const int status, AxioHeader* headers, int headerAmount, MemoryPool *responsePool);

#endif // AXIONETD_REQUEST_H