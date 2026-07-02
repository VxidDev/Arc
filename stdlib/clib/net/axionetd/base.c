#include "include/axionetd.h"
#include "include/http.h"
// #include "include/json.h"
#include "include/memory-pool.h"
#include "include/router.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void root(AxioRequest* request, AxioResponse* response, MemoryPool* responsePool) {
    /*
    char resp[128];

    if (AxioRequest_parseJSON(request)) {
        const char *name = "Default";
        AxioRequest_JSONGetStr(request, "name", &name);

        int age = 0;
        AxioRequest_JSONGetInt(request, "age", &age);
       
        bool isAlive = false;
        AxioRequest_JSONGetBool(request, "isAlive", &isAlive);

        snprintf(resp, sizeof(resp), "<h1>Hello, %s! Your age: %d. You're %s!</h1>", name, age, isAlive ? "alive" : "dead");
        HTMLResponse(response, resp, 200, NULL, 0);
    
        return;
    }
    */

    HTMLResponse(response, "<h1>Hello, World!</h1>", 200, NULL, 0, responsePool);
}

void diffPath(AxioRequest* request, AxioResponse* response, MemoryPool *responsePool) {
    sleep(5);
    HTMLResponse(response, "<h1>Done!</h1>", 200, NULL, 0, responsePool);
}

int main(void) {
    Axionet* server = initServer("0.0.0.0", 8000, 8192, 4, false);

    if (!server) {
        printf("Failed to create server.\n");
        return 1;
    }

    AxioRoute route;
    addRoute(server, "/", (char*[]){}, 0, root, &route, false);

    AxioRoute route2;
    addRoute(server, "/heavy", (char*[]){}, 0, diffPath, &route2, true);

    startServer(server);
    free(server);
    return 0;
}
