#ifndef AXIONET_JSON_H
#define AXIONET_JSON_H

#include <yyjson.h>
#include "http.h"

yyjson_val* AxioRequest_JSONGetObj(AxioRequest *request, const char* key);

bool AxioRequest_JSONGetStr(AxioRequest* request, const char* key, const char **out);
bool AxioRequest_JSONGetInt(AxioRequest* request, const char* key, int *out);
bool AxioRequest_JSONGetBool(AxioRequest* request, const char* key, bool *out);

#endif // AXIONET_JSON_H