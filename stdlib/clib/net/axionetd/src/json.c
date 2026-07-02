#include "../include/json.h"
#include "../include/axionetd.h"

#include <yyjson.h>

bool _ensure_jsonRoot(AxioRequest *request) {
    if (!request || !request->json)
        return false;

    if (!request->jsonRoot) {
        request->jsonRoot = yyjson_doc_get_root(request->json);
    }

    return request->jsonRoot != NULL;
}

yyjson_val* AxioRequest_JSONGetObj(AxioRequest *request, const char *key) {
    if (!request || !key) return NULL;

    if (!_ensure_jsonRoot(request) || !request->jsonRoot) return NULL;

    return yyjson_obj_get(request->jsonRoot, key);
}

bool AxioRequest_JSONGetStr(AxioRequest *request, const char *key, const char **out) {
    if (!request || !key || !out) return false;

    if (!_ensure_jsonRoot(request) || !request->jsonRoot) return false;

    yyjson_val *val = yyjson_obj_get(request->jsonRoot, key);
    if (!val || !yyjson_is_str(val)) return false;

    *out = yyjson_get_str(val);
    return true;
}

bool AxioRequest_JSONGetInt(AxioRequest *request, const char *key, int *out) {
    if (!request || !key || !out) return false;

    if (!_ensure_jsonRoot(request) || !request->jsonRoot) return false;

    yyjson_val *val = yyjson_obj_get(request->jsonRoot, key);
    if (!val || !yyjson_is_int(val)) return false;

    *out = yyjson_get_int(val);
    return true;
}

bool AxioRequest_JSONGetBool(AxioRequest *request, const char *key, bool *out) {
    if (!request || !key || !out) return false;

    if (!_ensure_jsonRoot(request) || !request->jsonRoot) return false;

    yyjson_val *val = yyjson_obj_get(request->jsonRoot, key);
    if (!val || !yyjson_is_bool(val)) return false;

    *out = yyjson_get_bool(val);
    return true;
}