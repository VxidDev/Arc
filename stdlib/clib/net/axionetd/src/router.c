#include "../include/axionetd.h"
#include "../include/router.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool addRoute(Axionet *server, char *path, char **methods, size_t amountOfMethods, void (*handler)(AxioRequest *, AxioResponse *, axio_MemoryPool *, void *), AxioRoute *route, bool threaded) {
        if (!server || !path || !handler) {
            return false;
        }
    
        if (!route) { // Handle NULL pointer
            return false; 
        }
    
        // Fill in route data
        size_t i;
    
        for (i = 0; i < AXIO_MAX_PATH - 1 && path[i]; i++) {
            route->path[i] = path[i];
        }
    
        route->path[i] = '\0';
    
        route->handler = handler;
    
        if (server->routeAmount >= server->routeCapacity) {
            size_t newCap = server->routeCapacity == 0 ? 4 : server->routeCapacity * 2;
    
            AxioRoute** temp = realloc(server->routes, sizeof(AxioRoute*) * newCap);
    
            if (!temp) {
                return false;
            }
    
            server->routes = temp;
            server->routeCapacity = newCap;
        }
    
        memset(route->methods, 0, sizeof(route->methods));
    
        for (size_t i = 0; i < amountOfMethods; i++) {
            route->methods[i] = methods[i];
        }
    
        route->amountOfMethods = amountOfMethods;
        route->threaded = threaded;
    
        server->routes[server->routeAmount++] = route;
    
        return true; }
