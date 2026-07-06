#ifndef AXIONETD_ROUTER_H
#define AXIONETD_ROUTER_H

#include "axionetd.h"
#include "config.h"
#include "http.h"
#include "memory-pool.h"

#include "../../../../../include/object.h"

typedef struct AxioRoute {
    char path[AXIO_MAX_PATH]; // Path, e.g "/"
    char *methods[AXIO_MAX_METHODS]; // Array of methods route allows or NULL as a wildcard
    int amountOfMethods;
    void (*handler)(AxioRequest *, AxioResponse *, axio_MemoryPool *, void *); // Handler which should return a HTTP response.
    bool threaded;
    void* userdata;
} AxioRoute;

bool addRoute(Axionet* server, char* path, char **methods, size_t amountOfMethods, void (*handler)(AxioRequest *, AxioResponse *, axio_MemoryPool *, void *userdata), AxioRoute *route, bool threaded);

#endif // AXIONETD_ROUTER_H
