#include "../include/http.h"
#include "../include/memory-pool.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

bool _extract_method(AxioRequest *request) {
    int i = 0;

    // copy until space or limit - 1
    while (request->raw[i] != ' ' && request->raw[i] != '\0' && i < AXIO_MAX_METHOD - 1) {
        request->method[i] = request->raw[i];
        i++;
    }

    request->method[i] = '\0';

    request->raw += i;

    return (i > 0); // success only if something was extracted
}

bool _extract_path(AxioRequest *request) {
    if (!request->raw) return false;

    request->raw++;

    int i = 0;

    // copy path until next space
    while (*request->raw != ' ' && *request->raw != '\0' && i < AXIO_MAX_PATH - 1) {
        request->path[i++] = *request->raw++;
    }

    request->path[i] = '\0';
    request->raw += i;
    return (i > 0);
}

bool _extract_headers(AxioRequest* request) {
    char *p = strstr(request->raw, "\r\n");
    if (!p) return false;

    p += 2; // skip "\r\n"

    char *end = strstr(p, "\r\n\r\n");
    if (end) *end = '\0';

    int count = 0;
    
    while (*p && count < AXIO_MAX_HEADERS) {
        char *line = p;

        // Find end of line
        while (*p && !(p[0] == '\r' && p[1] == '\n')) p++;

        if (*p) {
            *p = '\0';
            p += 2;
        }

        // Find colon
        char *colon = line;
        while (*colon && *colon != ':') colon++;

        if (*colon == ':') {
            *colon = '\0';

            char *value = colon + 1;
            if (*value == ' ') value++;

            request->headers[count].key = line;
            request->headers[count].value = value;

            if (!request->contentTypeSet && strcmp(request->headers[count].key, "Content-Type") == 0) {
                int i = 0;

                for (; *line && i < 64; i++) {
                    request->contentType[i] = value[i];
                } 

                request->contentTypeSet = true;
                request->contentType[i] = '\0';
            }

            count++;
        } 
    }
    
    request->headerAmount = count;
    return true;
} 

void _extract_queryString(AxioRequest* request) {
    char *query = strchr(request->path, '?');

    if (query) {
        *query = '\0';
        request->queryString = query + 1;
    } else {
        request->queryString = NULL;
    }
}

bool _parse_query_params(AxioRequest* request, MemoryPool *queryPool) {
    if (!request->queryString) {
        request->queryParams = NULL;
        request->queryParamAmount = 0;
        return true;
    }

    request->queryParams = poolAlloc(queryPool);
    request->queryParamAmount = 0;

    char *p = request->queryString;

    while (*p) {
        if (*p == '&') {  // skip empty params
            p++;
            continue;
        }

        if (request->queryParamAmount >= AXIO_MAX_QUERY_PARAMS)
            return false;

        char *eq = strchr(p, '=');
        char *amp = strchr(p, '&');

        if (!eq || (amp && eq > amp)) {
            // key without value
            return false;
        }

        int klen = eq - p;
        if (klen >= AXIO_MAX_QUERY_KEY_LEN) return false;

        memcpy(request->queryParams[request->queryParamAmount].key, p, klen);
        request->queryParams[request->queryParamAmount].key[klen] = '\0';
        request->queryParams[request->queryParamAmount].klen = klen;

        char *value = eq + 1;

        int vlen = amp ? (amp - value) : strlen(value);
        if (vlen >= AXIO_MAX_QUERY_VALUE_LEN) return false;

        memcpy(request->queryParams[request->queryParamAmount].value, value, vlen);
        request->queryParams[request->queryParamAmount].value[vlen] = '\0';
        request->queryParams[request->queryParamAmount].vlen = vlen;

        // printf("%s = %s\n", request->queryParams[request->queryParamAmount].key, request->queryParams[request->queryParamAmount].value);

        request->queryParamAmount++;

        if (!amp)
            break;

        p = amp + 1;
    }

    return true;
}


bool AxioRequest_parseJSON(AxioRequest* request) {
    if (strcmp(request->contentType, "application/json") != 0) {
        return false;
    }

    char *body = request->headers[request->headerAmount - 1].value + strlen(request->headers[request->headerAmount - 1].value); // Beginning of body
    if (!body) return false;

    body += 4;

    request->json = yyjson_read(body, strlen(body), 0);

    return request->json != NULL;
}

bool parseRequest(AxioRequest *request, char *buf, MemoryPool *queryPool) {
    if (!request) {
        return false;
    }

    request->contentTypeSet = false;
    request->raw = buf;

    if (!_extract_method(request)) { // Load method (e.g "GET") to request->method or handle error
        return false;
    }

    if (!_extract_path(request)) { // Load path (e.g "/") to request->path
        return false;
    }

    if (!_extract_headers(request)) { // fills in request->headers
        return false;
    }
    
    _extract_queryString(request);

    if (!_parse_query_params(request, queryPool)) { // fills in request->queryParams
        return false;
    }

    request->json = NULL; // Initialize
    request->jsonRoot = NULL; // Initialize

    return true;
}

void initResponse(AxioResponse *resp, const char* body, int status, AxioHeader* headers, int headerCount, MemoryPool *responsePool) {
    size_t bodyLen = body ? strlen(body) : 0;
    
    // compute size manually 
    size_t needed = 0;
    
    // status line (estimate fixed format)
    needed += 32; // "HTTP/1.1 200\r\n" + safety margin
    needed += 18; // "Content-Length: " + digits + "\r\n"
    needed += 22; // "Connection: close\r\n"
    
    for (int i = 0; i < headerCount; i++) {
        size_t klen = headers[i].klen ? headers[i].klen : strlen(headers[i].key);
        size_t vlen = headers[i].vlen ? headers[i].vlen : strlen(headers[i].value);
        needed += klen + vlen + 4; // ": \r\n"
    }
    
    needed += 2; // "\r\n" (blank line between headers and body)
    needed += bodyLen;
    needed += 1; // null terminator
    
    if (needed == 0 || needed >= 16384) {
        resp->response = NULL;
        resp->status = 500;
        resp->len = 0;
        return;
    }
    
    char *response = poolAlloc(responsePool);

    if (!response) {
        resp->response = NULL;
        resp->status = 500;
        resp->len = 0;
        return;
    }
    
    char *p = response;
    
    // Write status line
    memcpy(p, "HTTP/1.1 ", 9); 
    p += 9;
    
    // Write status code 
    int statusCode = status;
    if (statusCode < 100) statusCode = 500;
    if (statusCode > 999) statusCode = 500;
    
    p[0] = '0' + (statusCode / 100);
    p[1] = '0' + ((statusCode / 10) % 10);
    p[2] = '0' + (statusCode % 10);
    p += 3;
    
    memcpy(p, "\r\n", 2);
    p += 2;
    
    // Write Content-Length header
    memcpy(p, "Content-Length: ", 16);
    p += 16;
    
    // Convert body length to decimal string
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%zu", bodyLen);
    if (len > 0) {
        memcpy(p, buf, len);
        p += len;
    }
    
    memcpy(p, "\r\n", 2);
    p += 2;
    
    //write Connection header
    memcpy(p, "Connection: close\r\n", 19);
    p += 19;
    
    // Write custom headers
    for (int i = 0; i < headerCount; i++) {
        const char* key = headers[i].key;
        const char* value = headers[i].value;
        
        // Skip NULL headers
        if (!key || !value) continue;
        
        size_t klen = headers[i].klen ? headers[i].klen : strlen(key);
        size_t vlen = headers[i].vlen ? headers[i].vlen : strlen(value);
        
        memcpy(p, key, klen);
        p += klen;
        *p++ = ':';
        *p++ = ' ';
        memcpy(p, value, vlen);
        p += vlen;
        *p++ = '\r';
        *p++ = '\n';
    }
    
    // Write blank line
    p[0] = '\r';
    p[1] = '\n';
    p += 2;
    
    // Write body
    if (bodyLen > 0) {
        memcpy(p, body, bodyLen);
        p += bodyLen;
    }
    
    // Null terminate
    *p = '\0';
    
    // Set response fields
    resp->response = response;
    resp->len = (size_t)(p - response);
    resp->status = status;
}


void HTMLResponse(AxioResponse* resp, const char* body, const int status, AxioHeader* headers, int headerAmount, MemoryPool *responsePool) {
    AxioHeader h[headerAmount + 1];

    for (int i = 0; i < headerAmount; i++) {
        h[i] = headers[i];
    }

    char* key = "Content-Type";
    char* value = "text/html";
    h[headerAmount] = (AxioHeader){key, value, strlen(key), strlen(value)};

    initResponse(resp, body, status, h, headerAmount, responsePool);
}