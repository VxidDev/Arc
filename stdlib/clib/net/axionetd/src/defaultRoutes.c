#include "../include/defaultRoutes.h"
#include "../include/http.h"

void route404(AxioResponse *resp, MemoryPool *responsePool) {
    HTMLResponse(resp, "<h1>404 Not Found</h1>", 404, NULL, 0, responsePool); 
}

void route405(AxioResponse *resp, MemoryPool *responsePool) {
    HTMLResponse(resp, "<h1>405 Method Not Allowed</h1>", 405, NULL, 0, responsePool);
}

void route400(AxioResponse *resp, MemoryPool *responsePool) {
    HTMLResponse(resp, "<h1>400 Bad Request</h1>", 400, NULL, 0, responsePool);
}