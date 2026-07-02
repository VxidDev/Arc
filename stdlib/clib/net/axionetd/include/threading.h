#ifndef AXIO_THREADING
#define AXIO_THREADING

#include "http.h"

typedef struct {
    AxioConnection *conn;
    AxioRequest request;
} AxioJob;

typedef struct {
    AxioJob jobs[AXIO_JOB_QUEUE_SIZE];
    int front, back, size;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
} AxioJobQueue;

#endif // AXIO_THREADING